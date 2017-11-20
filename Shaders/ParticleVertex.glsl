#version 330 core

in layout(location = 0) vec3 modelPosition;	// Quad vertex positions
in layout(location = 1) vec4 xyzs;		// Position of particle centre and particle size
in layout(location = 2) vec4 colour;		// RGBA

out vec2 texCoords;
out vec4 particleColour;

uniform vec3 cameraUp;		// Camera position in world space
uniform vec3 cameraRight;	// Camera right in world space
uniform mat4 viewProjMatrix;// Camera view projection matrix

void main() {

	// TODO: Change this horrible notation
	float particleSize = xyzs.w;
	vec3 particleCentre = xyzs.xyz; // Particle centre in world space

	// Vertex position in world space
	vec3 vertexPosition = particleCentre +
		cameraRight * modelPosition.x * particleSize +
		cameraUp * modelPosition.y * particleSize;

	gl_Position = viewProjMatrix * vec4(vertexPosition, 1.0f);

	particleColour = colour;
	// Model position defined from [-0.5, 0.5] so map it to the range [0, 1]
	texCoords = modelPosition.xy + vec2(0.5);         
}