#pragma once
#include<GL/glew.h>
#include<string_view>
#include<vector>

namespace gl_stuff {
namespace detail {

	//https://www.khronos.org/opengl/wiki/OpenGL_Error
	void GLAPIENTRY
	GlMessageCallback( GLenum source,
   	              GLenum type,
   	              GLuint id,
   	              GLenum severity,
   	              GLsizei length,
  								const GLchar* message,
  								const void* userParam );

	void init_glew();

	struct gl_program {

		struct shader {
			GLuint shader_id;
			char const * name;
			bool free {true};
			shader(char const * name, std::string_view const source, GLuint program_id, GLenum shader_type);
			shader(shader const &) = delete;
			shader(shader && b) :
				shader_id{b.shader_id},
				name{b.name},
				free{b.free} { b.free = false; }
			~shader(){
				if(free){
					glDeleteShader(shader_id);
				}
			}
		};

		GLuint program_id;
		char const* name;
		std::vector<shader> shaders;

		gl_program(char const * name) : 
			program_id{glCreateProgram()},
			name{name} {};
		gl_program(gl_program const &) = delete;
		gl_program(gl_program&& p) : program_id{p.program_id}, name{p.name}, shaders {std::move(p.shaders)}{
			p.program_id = 0;
		}
		~gl_program(){
			if(0 == program_id){
				glDeleteProgram(program_id);
			}
		}
		void add_shader(char const* name, std::string_view const source, GLenum shader_type);
		inline void use() { glUseProgram(program_id); }
		void link();
		inline GLint get_uniform(char const * name){
			return glGetUniformLocation(program_id, name);
		}
		inline GLint get_attrib(char const * name){
			return glGetAttribLocation(program_id, name);
		}
	};

	struct gl_buffer {
		GLuint buffer_id;
		bool free {true};
		gl_buffer();
		gl_buffer(gl_buffer const &) = delete;
		gl_buffer(gl_buffer&&);
		~gl_buffer(){
			if(free){
				glDeleteBuffers(1, &buffer_id);
			}
		}
		void buffer_data(std::size_t size, void const * data, GLenum buffer_type, GLenum usage);
		void bind(GLenum buffer_type);
	};

	struct gl_texture {
		GLuint texture_id;
		GLenum texture_type;
		bool free {true};
		gl_texture(GLenum texture_type);
		gl_texture(gl_texture const &) = delete;
		gl_texture(gl_texture &&);
		~gl_texture();
		void bind(GLenum texture_type);
		void load_from_memory(std::size_t width, std::size_t height, void * data);
		void load_single_channel_from_memory(std::size_t width, std::size_t height, void * data);
	};

	std::vector<unsigned char> load_texture_from_file(char const* name, int * width, int * height);

}
}

namespace gl_stuff{
	using detail::init_glew;
	using detail::gl_program;
	using detail::gl_buffer;
	using detail::gl_texture;
	using detail::load_texture_from_file;
}
