#version 150

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 texMatrix;

in vec3 position;
in vec2 texCoord;

out Vertex {
	vec2 transformedTexCoord;
	vec2 texCoord;
} OUT;

void main(void) {
	mat4 mvp = projMatrix * viewMatrix * modelMatrix;
	gl_Position = mvp * vec4(position, 1.0);
	OUT.transformedTexCoord = (texMatrix * vec4(texCoord, 0.0, 1.0)).xy;
	OUT.texCoord = texCoord;
}