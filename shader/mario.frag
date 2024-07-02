#version 460

out vec4 FragColor;

in vec3 fcolor;
in vec2 fuv;

uniform sampler2D tex0;

void main(){
	vec4 texColor = texture(tex0, fuv);
	vec3 mainColor = mix(fcolor, texColor.rgb, texColor.a);
	FragColor = vec4(mainColor, 1);
}