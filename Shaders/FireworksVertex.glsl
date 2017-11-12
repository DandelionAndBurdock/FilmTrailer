#version 330

layout (location = 0) in float inType;	   // Type of particle e.g. launcher, shell, secondary shell
layout (location = 1) in vec3 inPosition;  // Particle position
layout (location = 2) in vec3 inVelocity;  // Particle velocity
layout (location = 3) in float inLifetime; // Lifetime in milliseconds

out float type;
out vec3 position;
out vec3 velocity;
out float lifetime;

void main() {
	type = inType;
	position = inPosition;
	velocity = inVelocity;
	lifetime = inLifetime;
}