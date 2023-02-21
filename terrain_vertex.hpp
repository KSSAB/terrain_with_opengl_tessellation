#pragma once
#include<string_view>
namespace terrain_vertex{

constexpr std::string_view shader = R"(#version 430
in vec3 in_vertex;
uniform int terrain_width;
void main() {
	gl_Position = vec4(in_vertex, 1.0);
})";
}
