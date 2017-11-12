#include "ParticleManager.h"

#include "TextureManager.h"
#include "common.h"

#include <algorithm>


#include "../../glm/gtx/norm.hpp" // length2()

#include "ShaderManager.h"

using namespace Particles;
//TODO: Refactor
ParticleManager::ParticleManager()
{
	particles.reserve(MAX_PARTICLES);


	//TODO: Move to main loader
	texture = "Particle";
	TEXTURE_MANAGER->AddTexture(texture, TEXTUREDIR"particle.DDS");

	// Create and compile our GLSL program from the shaders
	SHADER_MANAGER->AddShader("Particle", SHADERDIR"ParticleVertex.glsl", SHADERDIR"ParticleFragment.glsl");
	SHADER_MANAGER->SetUniform("Particle", "diffuseTex", 0);

	lastUsedParticle = 0;

	positionBuffer = new GLfloat[FLOATS_PER_PARTICLE * MAX_PARTICLES];
	colourBuffer = new GLubyte[CHARS_PER_COLOUR * MAX_PARTICLES];

	glGenVertexArrays(1, &particleVAO);

	glGenBuffers(1, &billboardVBO);
	glBindBuffer(GL_ARRAY_BUFFER, billboardVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(billboardVerts), billboardVerts);

	glGenBuffers(1, &positionVBO);
	glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * FLOATS_PER_PARTICLE * sizeof(GL_FLOAT),
		nullptr, GL_STREAM_DRAW);

	glGenBuffers(1, &colourVBO);
	glBindBuffer(GL_ARRAY_BUFFER, colourVBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * CHARS_PER_COLOUR * sizeof(GLubyte),
		nullptr, GL_STREAM_DRAW);

	glBindVertexArray(particleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, billboardVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(particleVAO);

	glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, FLOATS_PER_PARTICLE, GL_FLOAT, GL_FALSE, 0, 0);

	
	glBindBuffer(GL_ARRAY_BUFFER, colourVBO);
	glEnableVertexAttribArray(2);
	// Note: Normalised is true: This will map our unsigned chars [0, 255] to floats in [0, 1]
	glVertexAttribPointer(2, CHARS_PER_COLOUR, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);

	// Set up instancing
	glVertexAttribDivisor(0, 0); // vertices : never update
	glVertexAttribDivisor(1, 1); // position : update per instance
	glVertexAttribDivisor(2, 1); // colour : update per instance   
}


void ParticleManager::Rebuffer() {

	glBindBuffer(GL_ARRAY_BUFFER, positionVBO);

	// In openGL allocating storage may be faster than the synchronization -> 	https://www.khronos.org/opengl/wiki/Buffer_Object_Streaming
	// Driver will return the memory block once it's not used.
	glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * FLOATS_PER_PARTICLE * sizeof(GLfloat), nullptr, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particles.size() * FLOATS_PER_PARTICLE * sizeof(GLfloat), &particles[0]);

	glBindBuffer(GL_ARRAY_BUFFER, colourVBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * CHARS_PER_COLOUR * sizeof(GLubyte),
		nullptr, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particles.size() * CHARS_PER_COLOUR * sizeof(GLubyte), &particles[0]);
}



ParticleManager::~ParticleManager()
{
	glDeleteVertexArrays(1, &particleVAO);
	glDeleteBuffers(1, &billboardVBO);
	glDeleteBuffers(1, &positionVBO);
	glDeleteBuffers(1, &colourVBO);

	delete[] colourBuffer;
	delete[] positionBuffer;
}

void ParticleManager::Render() {
	glBindVertexArray(particleVAO);
	SHADER_MANAGER->SetShader("Particle");

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	TEXTURE_MANAGER->BindTexture(texture);

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0,	4, particles.size()); //TODO: Remove magic numbers
}

// Returns index of the last used particle
GLuint ParticleManager::NextUnusedParticle()
{
	// Linear search is usually bad but since we are starting from last used particle should be efficient in most cases
	for (GLuint i = lastUsedParticle; i < particles.size(); ++i) {
		if (particles[i].lifeRemaining <= 0.0f) {
			lastUsedParticle = i;
			return i;
		}
	}
	auto iter = std::find_if(particles.cbegin(), particles.cend(), [](const Particle& p) { p.lifeRemaining <= 0.0f; });

	if (iter != particles.cend()) {
		lastUsedParticle = iter - particles.cbegin();
		return lastUsedParticle;
	}
	else { // All other particles still alive (Consider reducing lifetime or increasing MAX_PARTICLES)
		lastUsedParticle = 0; // Override first particle 
		return 0;
	}

}

void ParticleManager::Update(GLint msec, glm::vec3 cameraPos) {
	UpdateParticles(msec, cameraPos);
	Rebuffer();
	GenerateNewParticles(msec);
}

void ParticleManager::GenerateNewParticles(GLint msec) {
	int numNewParticles = msec * NEW_PARTICLES_PER_MS;
	// Cap number of new particles to 16ms (60 FPS) in case of frame rate spike
	const int MAX_NEW_PARTICLES = 16 * NEW_PARTICLES_PER_MS;
	numNewParticles = max(numNewParticles, MAX_NEW_PARTICLES);
}

void ParticleManager::UpdateParticles(GLint msec, glm::vec3 cameraPos) {

	for (int i = 0; i < particles.size(); ++i) {
		Particle& p = particles[i];
		p.lifeRemaining -= msec;

		GLfloat deltaTimeSec = msec / 1000.0f;

		if (p.lifeRemaining > 0.0f) {
			// Update "physics"
			p.vel += glm::vec3(0.0f, -9.81f, 0.0f) * deltaTimeSec;
			p.pos += p.vel * deltaTimeSec;
			p.cameraDistance = glm::length2(p.pos - cameraPos);

			positionBuffer[FLOATS_PER_PARTICLE * i + 0] = p.pos.x;
			positionBuffer[FLOATS_PER_PARTICLE * i + 1] = p.pos.y;
			positionBuffer[FLOATS_PER_PARTICLE * i + 2] = p.pos.z;
			positionBuffer[FLOATS_PER_PARTICLE * i + 3] = p.size;

			colourBuffer[CHARS_PER_COLOUR * i + 0] = p.colour[0];
			colourBuffer[CHARS_PER_COLOUR * i + 1] = p.colour[1];
			colourBuffer[CHARS_PER_COLOUR * i + 2] = p.colour[2];
			colourBuffer[CHARS_PER_COLOUR * i + 3] = p.colour[3];//TODO: Try fading out with time?

		}
		else {
			p.cameraDistance = -1.0f;
		}
	}
}