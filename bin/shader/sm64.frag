#version 460

out vec4 FragColor;

in vec2 fuv;

uniform sampler2D tex0;

void main(){
	vec4 texColor = texture(tex0, fuv);
	if(texColor.a < 0.5)discard;
	FragColor = texColor;
}