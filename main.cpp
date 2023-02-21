#include"gl.hpp"
#include"glfw_context.hpp"
#include"quad.hpp"

#include"terrain_vertex.hpp"
#include"terrain_tess_control.hpp"
#include"terrain_tess_eval.hpp"
#include"terrain_frag.hpp"

#include<glm/glm.hpp>
#include<glm/ext.hpp>
#define GLM_ENABLE_EXPERIMENTAL //needed for rotate_vector
#include<glm/gtx/rotate_vector.hpp>
#include<SOIL/SOIL.h>
#include<chrono>
#include<thread> //sleep_for
#include<iostream>

using terrain_size_t = int_least32_t;
constexpr terrain_size_t terrain_size { 127 };
constexpr std::enable_if< 0 != terrain_size % 2, terrain_size_t>::type tile_count {terrain_size * terrain_size};

static glm::vec3 & light_pos(){
	static glm::vec3 light_pos{0.0f, 0.0f, 1.0f};
	return light_pos;
}

static void wire_frame_toggle(){
	static bool wire_frame {false};
	if(wire_frame){
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else{
		glPolygonMode(GL_FRONT, GL_FILL);
	}
	wire_frame = !wire_frame;
}

static void fov_toggle(){
	static bool narrow {false};
	GLuint constexpr camera_fov_i = 1;
	if(narrow){
		glUniform1f(camera_fov_i, 0.6f);
	}
	else{
		glUniform1f(camera_fov_i, 1.0f);
	}
	narrow = !narrow;
}

void key_callback(glfw_context::glfw_window::window_ptr_t, int key, int scancode, int action, int mods ){
	switch (key){
		case GLFW_KEY_LEFT:
			if(GLFW_REPEAT == action){
				light_pos() = glm::rotate(light_pos(), glm::radians(-1.0f), glm::vec3{1.0f, 0.0f, 0.0f});
			}
			break;
		case GLFW_KEY_RIGHT:
			if(GLFW_REPEAT == action){
				light_pos() = glm::rotate(light_pos(), glm::radians(1.0f), glm::vec3{1.0f, 0.0f, 0.0f});
			}
			break;
		case GLFW_KEY_F:
			if(GLFW_PRESS == action){
				wire_frame_toggle();
			}
			break;
		case GLFW_KEY_V:
			if(GLFW_PRESS == action){
				fov_toggle();
			}
		default:
			return;
	}
}

int main(){

	std::srand(std::time(0));
	std::vector<unsigned char> heightmap (terrain_size * terrain_size * 8 * 8);
	for (std::size_t y {}; y < terrain_size * 8; ++y){
		for(std::size_t x {}; x < terrain_size * 8; ++x){
			float x_pos = static_cast<float>(x) / (terrain_size * 8.0f);
			float y_pos = static_cast<float>(y) / (terrain_size * 8.0f);
			std::size_t i = (y * terrain_size * 8) + x;
			unsigned char const total_pixel =
				static_cast<unsigned char>(glm::perlin(glm::vec2{x_pos, y_pos} * 2.0f) * 60.0f + 128.0f) +
				static_cast<unsigned char>(glm::perlin(glm::vec2{x_pos, y_pos} * 7.0f) * 40.0f);
			unsigned char const additional_noise = total_pixel +
				static_cast<unsigned char>(glm::perlin(glm::vec2{x_pos, y_pos} * 30.0f) * 23.0f) +
				static_cast<unsigned char>(glm::perlin(glm::vec2{x_pos, y_pos} * 72.0f) * 5.0f);

			heightmap[i] = glm::mix(
				total_pixel,
				additional_noise,
				std::pow(static_cast<float>(total_pixel) / (256.0f), 2.0f)
			);
		}
	}
	glfw_context::handle_errors();
	auto glfw_raii = glfw_context::glfw_raii();
	auto glfw_window = glfw_context::glfw_window(1920, 1080, "Adapative Tesselation Terrain");
	int window_height;
	int window_width;
	glfw_window.get_dimensions(&window_width, &window_height);

	auto glfw_context = glfw_context::glfw_context_scoped(glfw_window.window);
	glfwSetKeyCallback(glfw_window.window, key_callback);

	gl_stuff::init_glew();
	glEnable(GL_CULL_FACE);

	auto program = gl_stuff::gl_program("Terrain Renderer");

	auto quad = quad::generate_subdivided_quad_vertices(127, 127);
	auto vtx_buffer = gl_stuff::gl_buffer();
	constexpr GLenum vtx_buffer_type = GL_ARRAY_BUFFER;
	vtx_buffer.bind(vtx_buffer_type);
	vtx_buffer.buffer_data(sizeof(quad[0]) * quad.size(),
		quad.data(), vtx_buffer_type, GL_STATIC_DRAW);

	auto heightmap_elems = quad::generate_subdivided_quad_elements(127, 127);
	auto elem_buffer = gl_stuff::gl_buffer();
	constexpr GLenum element_buffer_type = GL_ELEMENT_ARRAY_BUFFER;
	elem_buffer.bind(element_buffer_type);
	elem_buffer.buffer_data(sizeof(heightmap_elems[0]) * heightmap_elems.size(),
		heightmap_elems.data(), element_buffer_type, GL_STATIC_DRAW);

	program.add_shader("Terrain VTX", terrain_vertex::shader, GL_VERTEX_SHADER);
	program.add_shader("Terrain Tess CTRL", terrain_tess_control::shader, GL_TESS_CONTROL_SHADER);
	program.add_shader("Terrain Tess EVAL", terrain_tess_eval::shader, GL_TESS_EVALUATION_SHADER);
	program.add_shader("Terrain FRAG", terrain_frag::shader, GL_FRAGMENT_SHADER);
	program.link();

	auto const terrain_model = program.get_uniform("m");
	auto const terrain_perspective = program.get_uniform("p");
	auto const terrain_view = program.get_uniform("v");
	auto const terrain_width = program.get_uniform("terrain_width");
	auto const sunlight_pos_uniform = program.get_uniform("light_position");

	auto const heightmap_uniform = program.get_uniform("heightmap");
	auto heightmap_buffer = gl_stuff::gl_texture(GL_TEXTURE_2D);
	heightmap_buffer.bind(GL_TEXTURE0);
	auto constexpr dimension = (terrain_size * 8);
	heightmap_buffer.load_single_channel_from_memory( dimension, dimension, heightmap.data());

	auto const sand_uniform = program.get_uniform("sand_tex");
	int sand_tex_width;
	int sand_tex_height;
	std::vector<unsigned char> sand_tex =
		gl_stuff::load_texture_from_file("sand.jpg", &sand_tex_width, &sand_tex_height);
	auto sand_buffer = gl_stuff::gl_texture(GL_TEXTURE_2D);
	sand_buffer.bind(GL_TEXTURE1);
	sand_buffer.load_from_memory(sand_tex_width, sand_tex_height, sand_tex.data());

	auto const grass_uniform = program.get_uniform("grass_tex");
	int grass_tex_width;
	int grass_tex_height;
	std::vector<unsigned char> grass_tex =
		gl_stuff::load_texture_from_file("grass.png", &grass_tex_width, &grass_tex_height);
	auto grass_buffer = gl_stuff::gl_texture(GL_TEXTURE_2D);
	grass_buffer.bind(GL_TEXTURE2);
	grass_buffer.load_from_memory(grass_tex_width, grass_tex_height, grass_tex.data());

	auto const rock_uniform = program.get_uniform("rock_tex");
	int rock_tex_width;
	int rock_tex_height;
	std::vector<unsigned char> rock_tex =
		gl_stuff::load_texture_from_file("rock.jpg", &rock_tex_width, &rock_tex_height);
	auto rock_buffer = gl_stuff::gl_texture(GL_TEXTURE_2D);
	rock_buffer.bind(GL_TEXTURE3);
	rock_buffer.load_from_memory(rock_tex_width, rock_tex_height, rock_tex.data());

	auto const vertex_attrib = program.get_attrib("in_vertex");

	program.use();
	glEnableVertexAttribArray(vertex_attrib);
	vtx_buffer.bind(GL_ARRAY_BUFFER);
	elem_buffer.bind(GL_ELEMENT_ARRAY_BUFFER);
	glVertexAttribPointer(vertex_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glUniform1i(terrain_width, static_cast<GLint>(terrain_size));
	glUniform1i(heightmap_uniform, 0);
	glUniform1i(sand_uniform, 1);
	glUniform1i(grass_uniform, 2);
	glUniform1i(rock_uniform, 3);
	fov_toggle();

	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_BLEND );

	while(!glfwWindowShouldClose(glfw_window.window)){
		static int count = 0;
		using stdy_clock = std::chrono::steady_clock;
		using time_unit = std::chrono::milliseconds;
		auto const begin_frame = stdy_clock::now();
		++ count;

		glfwPollEvents();

		glm::vec3 const light = light_pos();
		glUniform3f(sunlight_pos_uniform, light.x, light.y, light.z);
		constexpr glm::vec3 axis_z {0.0f, 0.0f, 1.0f};

		glm::mat4 const model =
			glm::rotate(glm::mat4{1.0f}, count/1000.0f, axis_z);
		glUniformMatrix4fv(terrain_model, 1, 0, glm::value_ptr(model));

		glm::vec3 const camera_pos = glm::vec3(63.5f, 0.0f, 30.0f);
		glm::mat4 const view =
			glm::lookAt(camera_pos, glm::vec3(0.0f, 0.0f, 0.0f), axis_z);
		glUniformMatrix4fv(terrain_view, 1, 0, glm::value_ptr(view));

		glm::mat4 const perspective =
			glm::perspective(45.0f, static_cast<float>(window_width)/window_height, 0.1f, 1000.0f);

		glUniformMatrix4fv(terrain_perspective, 1, 0, glm::value_ptr(perspective));

		glClearColor(0, 0, 0.5, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glPatchParameteri(GL_PATCH_VERTICES, 4);
		glDrawElements(GL_PATCHES, quad.size() * sizeof(quad[0]), GL_UNSIGNED_INT, 0);
		glfwSwapBuffers(glfw_window.window);

		auto const end_frame = stdy_clock::now();

		static time_unit sleep_adjustment {0}; //if we sleep, gpu still runs program, so we will add this
		//to cpu_time, cpu_time + sleep_adjustment = total frame time

		auto const cpu_time = std::chrono::duration_cast<time_unit>
			(end_frame - begin_frame);
		constexpr auto target_time = time_unit{32};
		constexpr auto min_time = time_unit{3};
		auto const frame_time = cpu_time + sleep_adjustment;
		if(frame_time > target_time){
			std::cout << "Frame time : " << frame_time.count() << "; Target : " << target_time.count() << '\n'; 
		}
		auto const fore_sleep = stdy_clock::now();
		std::this_thread::sleep_for(std::max((target_time - frame_time) - time_unit(3), min_time));
		auto const aft_sleep = stdy_clock::now();
		sleep_adjustment = std::chrono::duration_cast<time_unit>(aft_sleep - fore_sleep);
	}

	return 0;
}
