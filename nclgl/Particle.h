#pragma once

#include "../../GLEW/include/GL/glew.h"
#include "../../glm/glm.hpp"
struct Particle {
	glm::vec3 pos;	  // Position of particle
	glm::vec3 vel;	  // Velocity of particle
	GLint lifetime; // Lifetime in milliseconds

	Particle() : 
		pos(glm::vec3(0.0f)), vel(glm::vec3(0.0f)), lifetime(0) 
	{}
};


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
