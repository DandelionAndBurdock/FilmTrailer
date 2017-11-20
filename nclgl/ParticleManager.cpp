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
	particles = std::vector<Particle>(MAX_PARTICLES);
	positionBuffer = new GLfloat[MAX_PARTICLES * FLOATS_PER_PARTICLE];
	colourBuffer = new GLubyte[MAX_PARTICLES * CHARS_PER_COLOUR];

	//TODO: Move to main loader
	texture = "Particle";
	TEXTURE_MANAGER->AddTexture(texture, TEXTUREDIR"particle.png");

	// Create and compile our GLSL program from the shaders
	SHADER_MANAGER->AddShader("Particle", SHADERDIR"ParticleVertex.glsl", SHADERDIR"ParticleFragment.glsl");
	SHADER_MANAGER->SetUniform("Particle", "diffuseTex", 0);

	lastUsedParticle = 0;

	positionBuffer = new GLfloat[FLOATS_PER_PARTICLE * MAX_PARTICLES];
	colourBuffer = new GLubyte[CHARS_PER_COLOUR * MAX_PARTICLES];

	glGenVertexArrays(1, &particleVAO);
	glBindVertexArray(particleVAO);

	glGenBuffers(1, &billboardVBO);
	glBindBuffer(GL_ARRAY_BUFFER, billboardVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(billboardVerts), billboardVerts, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);


	glGenBuffers(1, &positionVBO);
	glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * FLOATS_PER_PARTICLE * sizeof(GL_FLOAT),
		nullptr, GL_STREAM_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, FLOATS_PER_PARTICLE, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &colourVBO);
	glBindBuffer(GL_ARRAY_BUFFER, colourVBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * CHARS_PER_COLOUR * sizeof(GLubyte),
		nullptr, GL_STREAM_DRAW);

	glEnableVertexAttribArray(2);
	// Note: Normalised is true: This will map our unsigned chars [0, 255] to floats in [0, 1]
	glVertexAttribPointer(2, CHARS_PER_COLOUR, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
	
	// Set up instancing
	glVertexAttribDivisor(0, 0); // vertices : never update
	glVertexAttribDivisor(1, 1); // position : update per instance
	glVertexAttribDivisor(2, 1); // colour : update per instance   

}


void ParticleManager::Rebuffer() {
	if (numParticles == 0) {
		return;
	}
	glBindVertexArray(particleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numParticles * FLOATS_PER_PARTICLE * sizeof(GLfloat), positionBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, colourVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numParticles * CHARS_PER_COLOUR * sizeof(GLubyte), colourBuffer);
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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	SHADER_MANAGER->SetShader("Particle");
	glBindVertexArray(particleVAO);
	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	TEXTURE_MANAGER->BindTexture(texture);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0,	4, numParticles); //TODO: Remove magic numbers
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
	auto iter = std::find_if(particles.cbegin(), particles.cend(), [](const Particle& p) { return p.lifeRemaining <= 0.0f; });

	if (iter != particles.cend()) {
		lastUsedParticle = iter - particles.cbegin();
		return lastUsedParticle;
	}
	else { // All other particles still alive (Consider reducing lifetime or increasing MAX_PARTICLES)
		lastUsedParticle = 0; // Override first particle 
		return 0;
	}

}

void ParticleManager::Update(GLfloat msec, glm::vec3 cameraPos) {
	UpdateParticles(msec, cameraPos);
	Rebuffer();
	GenerateNewParticles(msec);
}

void ParticleManager::GenerateNewParticles(GLfloat msec) {
	int numNewParticles = msec * NEW_PARTICLES_PER_MS;
	// Cap number of new particles to 16ms (60 FPS) in case of frame rate spike
	const int MAX_NEW_PARTICLES = 16 * NEW_PARTICLES_PER_MS;
	numNewParticles = min(numNewParticles, MAX_NEW_PARTICLES);
	for (int i = 0; i < numNewParticles; ++i) {
		int particleIndex = NextUnusedParticle();
		particles[particleIndex].lifeRemaining = 5000.0f;
		particles[particleIndex].pos = glm::vec3(0.0, 0.0, -20.0f);

		float spread = 1.5f;

		glm::vec3 mainDir = glm::vec3(0.0f, 10.0f, 0.0f);

		glm::vec3 randDir = glm::vec3( // [-1, 1]
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f
		);

		particles[particleIndex].vel = mainDir + randDir * spread;
		//particles[particleIndex].size = (rand() % 1000) / 100.0f + 20.0f; //TODO: Use RNG make better rand generation
		particles[particleIndex].size =100.0f; //TODO: Use RNG make better rand generation

		particles[particleIndex].colour[0] = (GLubyte) rand() % 256;
		particles[particleIndex].colour[1] = (GLubyte) rand() % 256;
		particles[particleIndex].colour[2] = (GLubyte) rand() % 256;
		particles[particleIndex].colour[3] = (GLubyte) (rand() % 256) / 3;

	}
}


void ParticleManager::UpdateParticles(GLfloat msec, glm::vec3 cameraPos) {
	numParticles = 0;
	for (int i = 0; i < particles.size(); ++i) {
		Particle& p = particles[i];
		p.lifeRemaining -= msec;

		GLfloat deltaTimeSec = msec / 1000.0f;

		if (p.lifeRemaining > 0.0f) {
			// Update "physics"
			p.vel += glm::vec3(0.0f, -9.81f, 0.0f) * deltaTimeSec;
			p.pos += p.vel * deltaTimeSec;
			p.cameraDistance = glm::length2(p.pos - cameraPos);

			positionBuffer[FLOATS_PER_PARTICLE * numParticles + 0] = p.pos.x;
			positionBuffer[FLOATS_PER_PARTICLE * numParticles + 1] = p.pos.y;
			positionBuffer[FLOATS_PER_PARTICLE * numParticles + 2] = p.pos.z;
			positionBuffer[FLOATS_PER_PARTICLE * numParticles + 3] = p.size;

			colourBuffer[CHARS_PER_COLOUR * numParticles + 0] = p.colour[0];
			colourBuffer[CHARS_PER_COLOUR * numParticles + 1] = p.colour[1];
			colourBuffer[CHARS_PER_COLOUR * numParticles + 2] = p.colour[2];
			colourBuffer[CHARS_PER_COLOUR * numParticles + 3] = p.colour[3];
			++numParticles;
		}
		else {
			p.cameraDistance = -1.0f;
		}
	}
}