#include"gl.hpp"
#include<GL/glew.h>
#include<iostream>
#include<SOIL/SOIL.h>

namespace gl_stuff {
namespace detail {
  //https://www.khronos.org/opengl/wiki/OpenGL_Error
  void GlMessageCallback( GLenum /*source*/,
                  GLenum type,
                  GLuint /*id*/,
                  GLenum severity,
                  GLsizei /*length*/,
                  const GLchar* message,
                  const void* /*userParam*/ )
  {
    std::cerr << "GL CALLBACK type : "
      << ((GL_DEBUG_TYPE_ERROR == type)? "** GL ERROR **" : "NON ERROR")
      << "\nseverity : "
      << ((GL_DEBUG_SEVERITY_HIGH == severity || GL_DEBUG_SEVERITY_MEDIUM == severity)? "HIGH" : "LOW") 
      << "\nmessage : " << message << std::endl;
  }

	void init_glew(){
		GLenum err = glewInit();
		if(GLEW_OK != err){
			std::cerr << "Error : " << glewGetErrorString(err) << '\n';
		}
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback( GlMessageCallback, 0);
	}

	std::vector<unsigned char> load_texture_from_file(char const * filename, int * width, int * height){
		int channels;
		unsigned char* chars = SOIL_load_image(filename, width, height, &channels, SOIL_LOAD_RGBA);
		//channels arg is not populated correctly when force channels is used, so we manually put 4
		std::vector<unsigned char> ret_value (chars, chars + ((*width) * (*height) * 4));
		SOIL_free_image_data(chars);
		return ret_value;
	}

	//gl_program
	gl_program::shader::shader(char const * name, std::string_view const source, GLuint program_id, GLenum shader_type) :
			shader_id{glCreateShader(shader_type)},
			name {name} {
		auto const source_addr = &source[0];
		glShaderSource(shader_id, 1, &source_addr, 0);
		glCompileShader(shader_id);
		bool const good_compile = [this](){
					GLint compile;
					glGetShaderiv(this->shader_id, GL_COMPILE_STATUS, &compile);
					return (GL_TRUE == compile);
				}();
		std::cout << "\nShader: " << name << "\nSource:\n" << source << "\nCompiles? "
					<< (good_compile? "YES\n" : "NO\n");
		if(good_compile){
			glAttachShader(program_id, shader_id);
		}
	}

	void gl_program::add_shader(char const* name, std::string_view const source, GLenum shader_type){
		shaders.push_back({name, source, program_id, shader_type});
	}

	void gl_program::link(){
		glLinkProgram(program_id);
		bool const good_link = [this](){
			GLint status;
			glGetProgramiv(this->program_id, GL_LINK_STATUS, &status);
			return (status == GL_TRUE);
		}();
		std::cout << "\nProgram: " << name << "\nLink Status " << (good_link? "YES\n" : "NO\n");
	}
	//end gl_program

	//gl_buffer
	gl_buffer::gl_buffer() :
		buffer_id{	[](){ GLuint id; glGenBuffers(1, &id); return id; }() }
	{}

	gl_buffer::gl_buffer(gl_buffer&& b) :
		buffer_id{b.buffer_id},
		free{b.free}
	{
		b.free = false;
	}

	void gl_buffer::bind(GLenum buffer_type){
		glBindBuffer(buffer_type, buffer_id);
	}

	void gl_buffer::buffer_data(std::size_t size, void const * data, GLenum buffer_type, GLenum usage){
		glBufferData(buffer_type, size, data, usage);
	}
	//end gl_buffer

	//gl_texture
	gl_texture::gl_texture(GLenum texture_type) :
		texture_id { [](){ GLuint id; glGenTextures(1, &id); return id; }() },
		texture_type {texture_type}
	{}

	gl_texture::gl_texture(gl_texture && t) :
		texture_id {t.texture_id},
		texture_type {t.texture_type},
		free {t.free}
	{
		t.free = false;
	}

	gl_texture::~gl_texture(){
		if(free){
			glDeleteTextures(1, &texture_id);
		}
	}

	void gl_texture::bind(GLenum texture_unit){
		glActiveTexture(texture_unit);
		glBindTexture(texture_type, texture_id);
		glTexParameterf(texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	void gl_texture::load_from_memory(std::size_t width, std::size_t height, void * data){
		glTexImage2D(texture_type, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	void gl_texture::load_single_channel_from_memory(std::size_t width, std::size_t height, void * data){
		glTexImage2D(texture_type, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
	}

	//end gl_texture
}
}

namespace gl_stuff{

}

