#version 150 core

uniform sampler2D diffuseTex;

in vec2 texCoords;
in vec4 particleColour;

out vec4 gl_FragColor;

void main(){
	gl_FragColor = texture(diffuseTex, texCoords) * particleColour;
}