#version 460

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in ivec4 bones;
layout (location = 3) in vec4 weights;
layout (location = 4) in vec3 morph_pos;

out vec2 fuv;

uniform mat4 wvp;

uniform Buffer{
	mat4 FinalTransform[1]; // Œã‚ÅŽ©“®‚ÅŠg’£‚³‚ê‚é
};

void main(){

	mat4 skinned = FinalTransform[bones[0]] * weights[0] +
				   FinalTransform[bones[1]] * weights[1] +
				   FinalTransform[bones[2]] * weights[2] +
				   FinalTransform[bones[3]] * weights[3] ;	

	gl_Position = wvp * skinned * vec4(position + morph_pos, 1);
	fuv = uv;
}