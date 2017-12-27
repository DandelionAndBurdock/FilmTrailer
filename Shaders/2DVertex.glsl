#version 330 core

in vec3 position;

uniform mat4 projMatrix;

void main() {
	gl_Position = projMatrix * vec4(position, 1.0);
}