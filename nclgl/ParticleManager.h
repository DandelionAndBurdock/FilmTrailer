//http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
//he first loop might look a little daunting.Because particles die over time we want to spawn nr_new_particles particles each frame, but since we've decided from the start that the total amount of particles we'll be using is nr_particles we can't simply push the new particles to the end of the list. This way we'll quickly get a list filled with thousands of particles which isn't really efficient considering only a small portion of that list has particles that are alive.

#pragma once

#include "../../GLEW/include/GL/glew.h"

#include <vector>

#include "Particle.h"

class ParticleManager
{
public:
	ParticleManager();
	~ParticleManager();

	
	void Update(GLint msec, glm::vec3 cameraPos);
	void Render();
protected:

	void Rebuffer();
	// Returns index of particle with lifeRemaining < 0 
	GLuint NextUnusedParticle();

	// Generate new particles
	void GenerateNewParticles(GLint msec);

	void UpdateParticles(GLint msec, glm::vec3 cameraPos);


	// Buffer for base quad
	GLuint billboardVBO;
	// Buffer for size and position of particles
	GLuint positionVBO;
	// VBO containing colour of the particles
	GLuint colourVBO;
	// Vertex Array Object
	GLuint particleVAO;

	// Billboard texture
	std::string texture;

	std::vector<Particles::Particle> particles;


	GLfloat* positionBuffer;
	GLubyte* colourBuffer;

	// Index of last u
	GLuint lastUsedParticle;
};

