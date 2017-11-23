#version 150 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

in  vec3 position;
in  vec4 colour;
in vec2 texCoord; // Indices

out Vertex	{
	vec4 colour;
	vec2 texIndices;
} OUT;

void main(void)	{
	mat4 mvp 		= projMatrix * viewMatrix * modelMatrix;
	gl_Position		= mvp * vec4(position, 1.0);
	OUT.colour		= colour;
	OUT.texIndices     = texCoord;
}