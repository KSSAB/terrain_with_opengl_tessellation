#pragma once
#include<GLFW/glfw3.h>
#include<utility> //std::move
#include<iostream>

namespace glfw_context {
namespace detail {

 	void error_handler(int error_code, const char* error){
		std::cerr << "GLFW Error code : " << error_code << ";\nError : "
			<< error << '\n';
	}
	void handle_errors() {
		glfwSetErrorCallback(error_handler);
	}

	struct glfw_raii {//calls init, terminate on destruction
		const bool success;
		glfw_raii() : success {static_cast<bool>(glfwInit())}{}
		~glfw_raii(){
			glfwTerminate();
		}
	};

	struct glfw_window {
		using window_ptr_t = GLFWwindow*;
		window_ptr_t window;

		glfw_window(int const width, int const height, const char* title) :
			window{glfwCreateWindow(width, height, title, nullptr, nullptr)} {}
		glfw_window(glfw_window&& move_from) : window { std::move(move_from.window) } {}
		glfw_window(glfw_window const &) = delete;
		inline void get_dimensions(int* width, int* height){
			glfwGetWindowSize(this->window, width, height);
		}
		~glfw_window(){
			if(nullptr != window){
				glfwDestroyWindow(window);
			}
		}
	};

	struct glfw_context_scoped {
		using window_ptr_t = GLFWwindow*;
		window_ptr_t old_window;

		glfw_context_scoped(window_ptr_t window) : 
				old_window { glfwGetCurrentContext() }{
			if(old_window != window){
				glfwMakeContextCurrent(window);
			}
		}

		~glfw_context_scoped() {
			const auto current = glfwGetCurrentContext();
			if(current != old_window){
				glfwMakeContextCurrent(old_window);
			}
		}
	};

}
}

namespace glfw_context {
	using detail::handle_errors;
	using detail::glfw_raii;
	using detail::glfw_window;
	using detail::glfw_context_scoped;
}
