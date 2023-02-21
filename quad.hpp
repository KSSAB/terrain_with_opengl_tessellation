#pragma once
#include<vector>
#include<glm/glm.hpp>
#include<GL/glew.h>

namespace quad{
namespace detail{
// top left
// front left
// front right
// rear right
	std::vector<glm::vec3> generate_subdivided_quad_vertices(std::size_t height, std::size_t width);

	std::vector<GLuint> generate_subdivided_quad_elements(std::size_t height, std::size_t width);
}
}

namespace quad{
	using detail::generate_subdivided_quad_vertices;
	using detail::generate_subdivided_quad_elements;
}
