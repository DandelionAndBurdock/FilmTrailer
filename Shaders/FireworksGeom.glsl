#version 150 core

layout(points) in;
layout(points) out;
layout(max_vertices = 30) out;

in float type[];
in vec3 position[];
in vec3 velocity[];
in float lifetime[];

// Note these must match Varyings in ParticleSystem.cpp
out float outType;
out vec3 outPosition;
out vec3 outVelocity;
out float outLifetime;

uniform float deltaTime; // Time between consecutive frames in milliseconds
uniform float time;		 // Total time since particle system was initialised in milliseconds (used as semi-random seed)
uniform sampler1D randomTex; // Texture for sampling random directions for secondary shells
								 //TODO: Add some noise
uniform float launcherLifetime; // Time  in milliseconds between launcher spawning new shell
uniform float shellLifetime;    // Time in milliseconds between shell creation and exploding into secondary shells
uniform float secondaryShellLifetime; // Time in milliseconds between secondary shell creation and removal from scene

// These should match definitions in Particle.h
#define FIREWORK_LAUNCHER 0.0f
#define FIREWORK_SHELL 1.0f
#define FIREWORK_SECONDARY_SHELL 2.0f

vec3 GetRandomDir(float texCoord)
{
	vec3 dir = texture(randomTex, texCoord).xyz;
	return (dir - vec3(0.5)); // randomTexture has values from 0 to 1 so transform answer to range -0.5 to 0.5 to get all directions

}

void main() {
	float age = lifetime[0] + deltaTime; 


	// Hnadle Launcher (Unfortunately have to branch on particle type and lifetime)
	if (type[0] == FIREWORK_LAUNCHER) {
		if (lifetime[0] >= launcherLifetime) {			// Time to output another shell to the buffer
			outType = FIREWORK_SHELL;
			outPosition = position[0];
			vec3 dir = GetRandomDir(time / 1000.0);
			dir.y = max(dir.y, 0.5);				// Cap y component so that always emitted upwards
			outVelocity = normalize(dir) / 20.0f;	//TODO: Remove random number + add noise
			outLifetime = 0.0;
			EmitVertex();
			EndPrimitive();
			age = 0.0;		// Reset launcher lifetime
		}
		// Output another launcher to the buffer so that shells continue to be spawn
		outType = FIREWORK_LAUNCHER;
		outVelocity = velocity[0];
		outPosition = position[0];
		outLifetime = age;
		EmitVertex();
		EndPrimitive();
	}
	else { // Handle shells and secondary shells
		   // Translate time into seconds for "physics" equations
		float deltaTimeSecs = deltaTime / 1000.0f;
		// Speed = distance / time
		vec3 deltaP = deltaTimeSecs * velocity[0];
		// Gravity
		vec3 deltaV = vec3(deltaTimeSecs) * vec3(0.0, -9.81, 0.0); // TODO: Remove magic number
		if (type[0] == FIREWORK_SHELL) {
			if (lifetime[0] < shellLifetime) { // Shell has not expired update its "physics"
				outType = FIREWORK_SHELL;
				outPosition = position[0] + deltaP;
				outVelocity = velocity[0] + deltaV;
				outLifetime = age;
				EmitVertex();
				EndPrimitive();
			}
			else { // BOOM!
				for (int i = 0; i < 10; i++) {//TODO: Make unifrm->Will need to change max_vertices
					outType = FIREWORK_SECONDARY_SHELL;
					outPosition = position[0];
					vec3 dir = GetRandomDir((time + i) / 1000.0f);
					outVelocity = normalize(dir) / 20.0;
					outLifetime = 0.0f;
					EmitVertex();
					EndPrimitive();
				}
			}
		}
		else {	// Secondary shells update physics if still alive or expire
			if (lifetime[0] < secondaryShellLifetime) {
				outType = FIREWORK_SHELL;
				outPosition = position[0] + deltaP;
				outVelocity = velocity[0] + deltaV;
				outLifetime = age;
				EmitVertex();
				EndPrimitive();
			}
		}

	}
}
