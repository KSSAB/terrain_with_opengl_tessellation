#pragma once
#include<string_view>

namespace terrain_frag{
constexpr std::string_view shader = R"(#version 430
uniform sampler2D heightmap;
uniform sampler2D sand_tex;
uniform sampler2D grass_tex;
uniform sampler2D rock_tex;
uniform vec3 light_position;
in vec2 coord;
out vec4 outcolor;

void main(){
	vec2 adj_coord = coord / 128.0f;
	float altitude = texture(heightmap, adj_coord).r;

	vec3 cumulative_normal;
	{//getting normal from texture
	//https://stackoverflow.com/questions/49640250/calculate-normals-from-heightmap
		vec2 coord_left = (coord + vec2(-1.0, 0.0)) / 128.0f;
		if(coord_left.x < 0.0f){
			coord_left.x = 0.0f;
		}
		float left_altitude = texture(heightmap, coord_left).r * 35.0f;//height is scaled 25x
		vec2 coord_right = (coord + vec2(1.0, 0.0)) / 128.0f;
		if(coord_right.x > 1.0f){
			coord_right.x = 1.0f;
		}
		float right_altitude = texture(heightmap, coord_right).r * 35.0f;
		vec2 coord_top = (coord + vec2(0.0, 1.0)) / 128.0f;
		if(coord_top.y > 1.0f){
			coord_top.y = 1.0f;
		}
		float top_altitude = texture(heightmap, coord_top).r * 35.0f;
		vec2 coord_below = (coord + vec2(0.0, -1.0)) / 128.0f;
		if(coord_below.y < 0.0f){
			coord_below.y = 0.0f;
		}
		float bot_altitude = texture(heightmap, coord_below).r * 35.0f;
		cumulative_normal = normalize(
			vec3(2.0f * (right_altitude - left_altitude),
				2.0f * (bot_altitude - top_altitude),
				-4.0f)
		);
	}
	cumulative_normal *= -1.0f;
	float angle_from_vert = degrees(
		acos(dot(cumulative_normal, vec3(0.0f, 0.0f, 1.0f)))
	);

	vec3 rocky_grass;
	vec3 rocky_sand;
	if(angle_from_vert > 53.0f){
		rocky_grass = texture(rock_tex, coord / 32.0f).xyz;
		rocky_sand = rocky_grass;
	}
	else if (angle_from_vert > 13.0f){
		float lerp_amount = (angle_from_vert - 13.0f) / 40.0f;
		rocky_grass = mix(
			texture(grass_tex, coord / 16.0f).xyz,
			texture(rock_tex, coord / 16.0f).xyz,
			lerp_amount
		);
		rocky_sand = mix(
			texture(sand_tex, coord / 32.0).xyz,
			texture(rock_tex, coord / 16.0f).xyz,
			lerp_amount
		);
	}
	else{
		rocky_grass = texture(grass_tex, coord / 16.0f).xyz;
		rocky_sand = texture(sand_tex, coord / 32.0f).xyz;
	}

	vec3 pre_shade;

  if(altitude < 0.4f){
		pre_shade = rocky_sand;
	}
	else if (altitude > 0.5f) {
		pre_shade = rocky_grass;
	}
	else {
		float lerp_amount = (altitude - 0.4f) * 10.0f;
		pre_shade = mix(
			rocky_sand,
			rocky_grass,
			lerp_amount
		);
	}
	//outcolor = pre_shade;
	vec3 light_intensity = vec3(
		0.7f,
		0.4f + max(light_position.z * 0.4f, 0),
		max(light_position.z, 0));
	float scalar = clamp( dot( normalize(light_position), cumulative_normal ), 0.0f, 1.0f);
	vec3 ambient = vec3(0.1f, 0.1f, 0.1f);
	vec3 final_outcolor = pre_shade * ((light_intensity * scalar) + ambient);
	outcolor = vec4(final_outcolor.xyz, 1.0f);
})";
}
