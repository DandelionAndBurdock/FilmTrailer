#version 150 core

in vec3 modelPosition;	// Quad vertex positions

out vec2 texCoords;

uniform vec3 cameraUp;		// Camera position in world space
uniform vec3 cameraRight;	// Camera right in world space
uniform mat4 viewProjMatrix;// Camera view projection matrix

// Having this as a uniform really limits the shader but I
// don't intend to use it very often (and time is pressing)
uniform vec3 particleCentre;		

void main() {
	const float size = 500.0f;

	// Vertex position in world space
	vec3 vertexPosition = particleCentre +
		cameraRight * modelPosition.x * size +
		cameraUp * modelPosition.y * size;

	gl_Position = viewProjMatrix * vec4(vertexPosition, 1.0f);

	// Model position defined from [-0.5, 0.5] so map it to the range [0, 1]
	texCoords = modelPosition.xy + vec2(0.5);         
}