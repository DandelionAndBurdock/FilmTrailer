#version 150

in vec3 pos;   			//Position in world space
in float thickness;

out float width;

void main() {
	width = thickness;
	gl_Position = vec4(pos, 1.0);
}