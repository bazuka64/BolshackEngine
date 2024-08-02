#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 uv;

out vec3 fcolor;
out vec2 fuv;

uniform mat4 mvp;

void main(){
	gl_Position = mvp * vec4(position, 1);
	fcolor = color;
	fuv = uv;
}