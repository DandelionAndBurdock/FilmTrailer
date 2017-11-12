#include "ParticleSystem.h"

#include "Particle.h"


#include "ShaderManager.h"
#include "TextureManager.h"
#include "Shader.h"

#include "common.h" //TODO: Remove

using namespace Fireworks;
ParticleSystem::ParticleSystem(const glm::vec3& pos)
{
	currentVBO = 0;
	currentFBO = 1;
	isFirst = true;
	time = 0;
	LoadShaders();
	//TODO: Move to texture load
	TEXTURE_MANAGER->AddTexture("Firework", TEXTUREDIR"firework.jpg");
	TEXTURE_MANAGER->GenerateRandomTexture("RandomTexture", 1000);

	InitFireworks(pos);

	glGenVertexArrays(1, &updateVAO);
	glGenVertexArrays(1, &billboardVAO);
}

ParticleSystem::~ParticleSystem()
{
	glDeleteVertexArrays(1, &updateVAO);
	glDeleteVertexArrays(1, &billboardVAO);
}


void ParticleSystem::InitFireworks(const glm::vec3& pos) {
	Firework fireworks[MAX_PARTICLES];
	// Initialise launcher
	fireworks[0].type = FIREWORK_LAUNCHER;
	fireworks[0].pos = pos;
	fireworks[0].vel = glm::vec3(0.0f);
	fireworks[0].lifetime = 0.0f;

	glGenTransformFeedbacks(2, transfromFeedBackBuffer);
	glGenBuffers(2, particleBuffer);

	for (GLuint i = 0; i < 2; ++i) {
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transfromFeedBackBuffer[i]);
		glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(fireworks), fireworks, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, particleBuffer[i]);
	}
}

void ParticleSystem::Render(const glm::mat4& viewProj, const glm::vec3 cameraPos) {
	RenderParticles(viewProj, cameraPos);

	currentVBO = currentFBO;
	currentFBO = ++currentFBO % 2;
	//currentFBO = (currentFBO + 1) & 0x1;
}

void ParticleSystem::UpdateParticles(GLint msec) {
	time += msec;
	// Configure shader
	SHADER_MANAGER->SetShader(updateShader);
	SHADER_MANAGER->SetUniform(updateShader, "time", time);
	SHADER_MANAGER->SetUniform(updateShader, "deltaTime", msec);

	glActiveTexture(GL_TEXTURE0);
	TEXTURE_MANAGER->BindTexture1D("RandomTexture");

	glBindVertexArray(updateVAO);
	glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[currentVBO]);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transfromFeedBackBuffer[currentFBO]);

	//  Set up buffer attributes //TODO: Do we need to do this every frame?

	glEnableVertexAttribArray(TYPE);											
	glVertexAttribPointer(TYPE, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), 0);
	glEnableVertexAttribArray(POSITION);
	glVertexAttribPointer(POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid*)sizeof(GL_FLOAT));
	glEnableVertexAttribArray(VELOCITY);
	glVertexAttribPointer(VELOCITY, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid*)(sizeof(GL_FLOAT) + sizeof(glm::vec3)));
	glEnableVertexAttribArray(LIFETIME);
	glVertexAttribPointer(LIFETIME, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid*)(sizeof(GL_FLOAT) + 2 * sizeof(glm::vec3)));

	// Next draw call will write to the TF buffer so 
	// don't want to spend time doing rasterisation
	glEnable(GL_RASTERIZER_DISCARD);

	// Redirect output to transform feedback buffer
	glBeginTransformFeedback(GL_POINTS);

	if (isFirst) { // Handle first draw call explicitly
		glDrawArrays(GL_POINTS, 0, 1);
		isFirst = false;
	}
	else {
		glDrawTransformFeedback(GL_POINTS, transfromFeedBackBuffer[currentFBO]);
	}

	// End of transform feedback
	glDisable(GL_RASTERIZER_DISCARD);
	glEndTransformFeedback();

	//TODO: Disable vertexattribarray?
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

}


void ParticleSystem::RenderParticles(const glm::mat4& viewProj, const glm::vec3 cameraPos) {
	SHADER_MANAGER->SetShader(billboardShader);
	SHADER_MANAGER->SetUniform(billboardShader, "cameraPos", cameraPos);
	SHADER_MANAGER->SetUniform(billboardShader, "viewProjMatrix", viewProj);
	
	glActiveTexture(GL_TEXTURE0);
	TEXTURE_MANAGER->BindTexture("Firework");


	//TODO: Why do we do this every frame try and move to init
	// Data is in the transform feedback buffer at currentFBO which is attached to the vertex buffer particleBuffer[currentFBO] 
	// Note: Only a single attribute (position) is needed for rendering
	glBindVertexArray(billboardVAO);
	glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[currentFBO]);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid*)sizeof(GL_FLOAT));

	glDrawTransformFeedback(GL_POINTS, transfromFeedBackBuffer[currentFBO]);

	glDisableVertexAttribArray(0);
}

// As we are using transform feedback and need to specify the buffers before linking we will not use the shader manager to load our shader
void ParticleSystem::LoadShaders() {
	updateShader = "FireworkShader";
	Shader* shader = new Shader(SHADERDIR"FireworksVertex.glsl", SHADERDIR"FireworksFrag.glsl", SHADERDIR"FireworksGeom.glsl");

	// Specify the attributes that will be written into the transform feedback buffer
	const GLchar* varyings[NUM_ATTRIBUTES];
	varyings[TYPE] = "outType";
	varyings[POSITION] = "outPosition";
	varyings[VELOCITY] = "outVelocity";
	varyings[LIFETIME] = "outLifetime";

	glTransformFeedbackVaryings(shader->GetProgram(), NUM_ATTRIBUTES, varyings, GL_INTERLEAVED_ATTRIBS);

	if (shader->LinkProgram()) {
		SHADER_MANAGER->AddShader(updateShader, shader);
	}
	else {
		return;
	}
	SHADER_MANAGER->SetUniform(updateShader, "randomTex", 0);
	SHADER_MANAGER->SetUniform(updateShader, "launcherLifetime", 100.0f);
	SHADER_MANAGER->SetUniform(updateShader, "shellLifetime", 10000.0f);
	SHADER_MANAGER->SetUniform(updateShader, "secondaryShellLifetime", 2500.0f);

	//TODO: Move this to normal load	
	billboardShader = "BillboardShader";
	SHADER_MANAGER->AddShader(billboardShader, SHADERDIR"BillboardVertex.glsl", SHADERDIR"BillboardFrag.glsl", SHADERDIR"BillboardGeom.glsl");
	SHADER_MANAGER->SetUniform(updateShader, "diffuseTex", 0);
}