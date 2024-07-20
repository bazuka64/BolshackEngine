#version 460

out vec4 FragColor;

in vec2 fuv;

uniform sampler2D tex0;

void main(){
	FragColor = texture(tex0, fuv);
}