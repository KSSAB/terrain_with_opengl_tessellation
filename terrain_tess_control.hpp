#pragma once
#include<string_view>

namespace terrain_tess_control{

constexpr std::string_view shader = R"(#version 430
layout(vertices = 4) out;
uniform mat4 p;
uniform mat4 v;
uniform mat4 m;
layout(location=1) uniform float fov;

int tess_for_pos(vec4 pos, mat4 mvp){
	float depth = -(mvp * pos).w / 12.0f;
	return clamp(int(8.0f + depth), 1, 8);
}

void zero_out_tess_level(){
	gl_TessLevelOuter[0] = 0;
	gl_TessLevelOuter[1] = 0;
	gl_TessLevelOuter[2] = 0;
	gl_TessLevelOuter[3] = 0;
	gl_TessLevelInner[0] = 0;
	gl_TessLevelInner[1] = 0;
}

void main(){
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	mat4 mvp = p * v * m;
	if(gl_InvocationID == 0){
		//frustrum cull
		{
			float cutoff = fov;

			vec4 test_pos_one = mvp * gl_in[0].gl_Position;
			bool cull = (test_pos_one.x < -cutoff * test_pos_one.w ||
				test_pos_one.x > cutoff * test_pos_one.w);

			vec4 test_pos_two = mvp * gl_in[1].gl_Position;
			cull = cull && (test_pos_two.x < -cutoff * test_pos_two.w ||
				test_pos_two.x > cutoff * test_pos_two.w);

			vec4 test_pos_three = mvp * gl_in[2].gl_Position;
			cull = cull && (test_pos_three.x < -cutoff * test_pos_three.w ||
				test_pos_three.x > cutoff * test_pos_three.w);

			vec4 test_pos_four = mvp * gl_in[3].gl_Position;
			cull = cull && (test_pos_four.x < -cutoff * test_pos_four.w ||
				test_pos_four.x > cutoff * test_pos_four.w);

			if(cull){
				zero_out_tess_level();
				return;
			}
		}
		//
		vec4 this_pos = gl_in[gl_InvocationID].gl_Position + vec4(0.5f, 0.5f, 0.0f, 0.0f);
		int tesslv = tess_for_pos(this_pos, mvp);
		//because mismatched edges cause seams, we will match up left and below to neighboring quad
		vec4 left_pos = (this_pos + vec4(-1.0f, 0.0f, 0.0f, 0.0f));
		int leftlv = tess_for_pos(left_pos, mvp);
		vec4 below_pos = (this_pos + vec4(0.0f, -1.0f, 0.0f, 0.0f));
		int belowlv = tess_for_pos(below_pos, mvp);
		//int tesslv = 1;
		gl_TessLevelOuter[0] = leftlv;
		gl_TessLevelOuter[1] = belowlv;
		gl_TessLevelOuter[2] = tesslv;
		gl_TessLevelOuter[3] = tesslv;
		gl_TessLevelInner[0] = tesslv;
		gl_TessLevelInner[1] = tesslv;
	}
})";

}

