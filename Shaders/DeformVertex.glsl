#version 150

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;


uniform vec2 epicentre;
uniform float holeTime;


in vec3 position;
in vec2 texCoord;

out Vertex {
	float height;
	vec2 texCoord;
} OUT;

// Basic Idea: Cubic interpolation (smoothstep) from 0 to epiHeight 
// distance 0 to raidus with both radius and epiHeight increasing with time
// Removed some stuff but out of time to fix without breaking
void main(void) {
	mat4 mvp = projMatrix * viewMatrix * modelMatrix;
	
	vec3 worldPos = vec3(modelMatrix * vec4(position, 1.0));
	float radius = 200.0f + holeTime /50.0f;
	float epiHeight = 100.0f + holeTime / 10.0f;
	
	float distanceFromEpi = length(worldPos.xz - epicentre);
	worldPos.y -= epiHeight * smoothstep(radius, 0.0, distanceFromEpi);
	
	
	gl_Position = projMatrix * viewMatrix * vec4(worldPos, 1.0);
	OUT.texCoord = texCoord;
	OUT.height = position.y / 350.0f; 
}