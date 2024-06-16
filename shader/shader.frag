#version 460

out vec4 color;

in vec2 fuv;

uniform sampler2D tex0;

void main(){
	color = texture(tex0, fuv);
}