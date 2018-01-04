#version 150

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform float time;

in vec3 position;
in vec2 texCoord;

out Vertex {
	vec2 texCoord;
} OUT;

void main(void) {
	mat4 mvp = projMatrix * viewMatrix * modelMatrix;
	gl_Position = mvp * vec4(position, 1.0);
	OUT.texCoord = vec2(texCoord.x, 1.0 - texCoord.y);

	float strength = 0.001;
	gl_Position.x += cos(time / 200) * strength;
	gl_Position.y += cos(time / 300) * strength;
}