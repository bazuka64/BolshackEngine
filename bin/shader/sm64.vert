#version 460

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

out vec2 fuv;

uniform mat4 wvp;

void main(){
	gl_Position = wvp * vec4(position, 1);
	fuv = uv;
}