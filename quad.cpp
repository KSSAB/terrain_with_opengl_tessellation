#include"quad.hpp"

namespace quad{
namespace detail{
	std::vector<glm::vec3> generate_subdivided_quad_vertices(std::size_t height, std::size_t width){
		std::vector<glm::vec3> ret_value ((height +1) * (width+1));
		float y_pos = -1.0f * (static_cast<float>(width) / 2.0f);
		const float base_x_pos = -1.0f * (static_cast<float>(height) / 2.0f);
		float x_pos = base_x_pos;
		for(std::size_t y {}; y < height + 1; ++y){
			for(std::size_t x {}; x < width + 1; ++x){
				static std::size_t i {0};
				ret_value[i] = {x_pos, y_pos, 0.0f};
				++i;
				x_pos += 1.0f;
			}
			y_pos += 1.0f;
			x_pos = base_x_pos;
		}
		return ret_value;
	}

	std::vector<GLuint> generate_subdivided_quad_elements(std::size_t height, std::size_t width){
		/*1 quad =
			n, //top left
			n + width //bottom left
			n + 1 + width //bottom right
			n + 1 // top right
			we do not shadow width with width+1 or height with height +1 as
			we access these elements by this algorithm anyway
			there will be four vertices * width * height
		*/
		std::vector<GLuint> ret_value (4 * width * height);
		for(std::size_t y{}; y < height; ++y){
			for(std::size_t x{}; x < width; ++x){
				std::size_t const top_left = (y * (width + 1)) + x;
				static std::size_t i {0};
				ret_value[i] = (top_left);
				ret_value[i + 1] = (top_left + width + 1);//not sure why off by one
				ret_value[i + 2] = (top_left + width + 2);//
				ret_value[i + 3] = (top_left + 1);
				i += 4;
			}
		}
		return ret_value;
	}
}
}

namespace quad{

}
