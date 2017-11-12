#pragma once

#include "../../GLEW/include/GL/glew.h"
#include "../../glm/glm.hpp"

namespace Particles {
	const int CHARS_PER_COLOUR = 4;  // r, g, b, a
	const int FLOATS_PER_PARTICLE = 4; // x, y, z and size
	struct Particle {
		glm::vec3 pos;	  // Position of particle
		glm::vec3 vel;	  // Velocity of particle
						  // Remaining life of the particle in milliseconds. if <0 : dead and unused.
		GLfloat lifeRemaining;
		GLfloat size;
		GLfloat angle;
		GLfloat weight;
		GLubyte colour[CHARS_PER_COLOUR];
		// Squared distance to camera used for sorting with transparency
		float cameraDistance;

		Particle() :
			pos(glm::vec3(0.0f)), vel(glm::vec3(0.0f)), lifeRemaining(0.0),
			size(0.0f), angle(0.0f), weight(0.0f), cameraDistance(-1.0f)
		{
			for (int i = 0; i < CHARS_PER_COLOUR; ++i) {
				colour[i] = 1;
			}
		}

		bool operator<(const Particle& rhs) const {
			// Reverse sort to draw particles farthest away from camera first
			return (this->cameraDistance > rhs.cameraDistance);
		}

	};


	// Vertices for a quad that can be textured //TODO: Could make 2D and save some memory?
	const GLfloat billboardVerts[] = {
		-0.5f, -0.5f, +0.0f,  // Bottom Left
		+0.5f, -0.5f, +0.0f,  // Bottom Right
		-0.5f, +0.5f, +0.0f,  // Top Left
		+0.5f, +0.5f, +0.0f,  // Top Right
	};
	

	const int MAX_PARTICLES = 100000;
	// The number of new particles to generate per millisecond
	const int NEW_PARTICLES_PER_MS = 10;
}





namespace Fireworks {
	// Define these as floats rather than enums since we will send them to a buffer
	const GLfloat INVALID_TYPE = -1.0f;
	const GLfloat FIREWORK_LAUNCHER = 0.0f;					// Static particle responsible for launching shells 
	const GLfloat FIREWORK_SHELL = 1.0f;					// Flies upwards before creating secondary shells
	const GLfloat FIREWORK_SECONDARY_SHELL = 2.0f;			// Moves in random direction and fades out

	// Note: I don't derive from firework from particle to ensure the memory layout does not have
	// any padding or unexpected
	struct Firework {
		GLfloat type;
		glm::vec3 pos;	  // Position of particle
		glm::vec3 vel;	  // Velocity of particle
		GLint lifetime; // Lifetime in milliseconds

		Firework() :
			type(INVALID_TYPE), pos(glm::vec3(0.0f)), vel(glm::vec3(0.0f)), lifetime(0)
		{}
	};
		
		const int MAX_PARTICLES = 1000;
		const float BASE_LIFETIME = 10.0f;

		// Layout in memory for Firework
		enum AttribIndexes {
			TYPE = 0,
			POSITION,
			VELOCITY,
			LIFETIME,
			NUM_ATTRIBUTES
		};
};
