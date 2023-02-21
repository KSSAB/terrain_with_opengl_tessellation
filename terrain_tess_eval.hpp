#pragma once
#include<string_view>

namespace terrain_tess_eval{

constexpr std::string_view shader = R"(#version 430
layout(quads, ccw) in;
uniform mat4 m;
uniform mat4 v;
uniform mat4 p;
uniform sampler2D heightmap;

//in vec2 tex_control_coord[];
out vec2 coord;

void main(){
	vec4 horizontal_one = mix(
		gl_in[0].gl_Position,
		gl_in[3].gl_Position,
		gl_TessCoord.x
	);
	vec4 horizontal_two = mix(
		gl_in[1].gl_Position,
		gl_in[2].gl_Position,
		gl_TessCoord.x
	);

	mat4 mvp = p * v * m;
	vec4 pos = mix(horizontal_one, horizontal_two, gl_TessCoord.y);

	coord = vec2(clamp(pos.x + 64, 0.0f, 128.0f), clamp(pos.y+ 64, 0.0f, 128.0f));
//apply heightmap
	vec2 adj_coord = coord / 128.0f;
	pos.z = texture(heightmap, adj_coord).r * 35.0f;

	gl_Position = mvp * pos;
})";

}

