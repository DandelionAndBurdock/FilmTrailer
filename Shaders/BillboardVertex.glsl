#version 150

in vec3 position;
in vec4 colour;
in float size;

out vec4 geomColour;
out float geomSize;

void main() {
	geomColour = colour;
	geomSize = size;
	gl_Position = vec4(position, 1.0);
}