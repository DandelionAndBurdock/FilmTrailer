#version 150 core

uniform sampler2D diffuseTex;

in vec2 texCoord;

out vec4 gl_FragColor;

void main(){
	gl_FragColor = texture(diffuseTex, texCoord);
}