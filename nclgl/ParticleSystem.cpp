#include "ParticleSystem.h"

#include "Particle.h"


#include "ShaderManager.h"
#include "TextureManager.h"
#include "Shader.h"

#include "common.h" //TODO: Remove
#include <iostream>
using namespace Fireworks;
FireworkSystem::FireworkSystem(const glm::vec3& pos)
{
	currentVBO = 0;
	currentFBO = 1;
	isFirst = true;
	time = 0.0;
	LoadShaders();
	//TODO: Move to texture load
	TEXTURE_MANAGER->AddTexture("Firework", TEXTUREDIR"particle.png");
	TEXTURE_MANAGER->GenerateRandomTexture("RandomTexture", 1000);

	InitFireworks(pos);

	glGenVertexArrays(1, &updateVAO);
	glGenVertexArrays(1, &billboardVAO);

	glBindVertexArray(updateVAO);
	glEnableVertexAttribArray(TYPE);
	glVertexAttribIPointer(TYPE, 1, GL_INT, sizeof(Firework), 0);
	glEnableVertexAttribArray(POSITION);
	glVertexAttribPointer(POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Firework), (GLvoid*)sizeof(GLint));
	glEnableVertexAttribArray(VELOCITY);
	glVertexAttribPointer(VELOCITY, 3, GL_FLOAT, GL_FALSE, sizeof(Firework), (GLvoid*)(sizeof(GLint) + sizeof(glm::vec3)));
	glEnableVertexAttribArray(LIFETIME);
	glVertexAttribPointer(LIFETIME, 1, GL_FLOAT, GL_FALSE, sizeof(Firework), (GLvoid*)(sizeof(GLint) + 2 * sizeof(glm::vec3)));
	glEnableVertexAttribArray(COLOUR);
	glVertexAttribPointer(COLOUR, 4, GL_FLOAT, GL_FALSE, sizeof(Firework), (GLvoid*)(2 * sizeof(GLint) + 2 * sizeof(glm::vec3)));
	glEnableVertexAttribArray(FIREWORK_SIZE);
	glVertexAttribPointer(FIREWORK_SIZE, 1, GL_FLOAT, GL_FALSE, sizeof(Firework), (GLvoid*)(2 * sizeof(GLint) + 2 * sizeof(glm::vec3) + sizeof(glm::vec4)));

	glBindVertexArray(billboardVAO);
}

FireworkSystem::~FireworkSystem()
{
	glDeleteVertexArrays(1, &updateVAO);
	glDeleteVertexArrays(1, &billboardVAO);
}


void FireworkSystem::InitFireworks(const glm::vec3& pos) {
	Firework fireworks[MAX_PARTICLES];
	// Initialise launcher
	fireworks[0].type = FIREWORK_LAUNCHER;
	fireworks[0].pos = pos;
	fireworks[0].vel = glm::vec3(0.0f, 1, 0);
	fireworks[0].lifetime = 0.0f;

	glBindVertexArray(updateVAO);

	glGenTransformFeedbacks(2, transfromFeedBackBuffer);
	glGenBuffers(2, particleBuffer);

	for (GLuint i = 0; i < 2; ++i) {
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transfromFeedBackBuffer[i]);

		glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, particleBuffer[i]);
		glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(fireworks), fireworks, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, particleBuffer[i]);


	}
	glBindVertexArray(0);
}

void FireworkSystem::Render(const glm::mat4& viewProj, const glm::vec3 cameraPos) {
	RenderParticles(viewProj, cameraPos);

	currentVBO = currentFBO;
	currentFBO = ++currentFBO % 2;
}

void FireworkSystem::UpdateParticles(GLfloat msec) {
	time += msec;

	glEnable(GL_RASTERIZER_DISCARD);

	GLuint query;
	glGenQueries(1, &query);
	

	// Configure shader
	SHADER_MANAGER->SetShader(updateShader);
	SHADER_MANAGER->SetUniform(updateShader, "time", time);
	SHADER_MANAGER->SetUniform(updateShader, "deltaTime", float(msec));

	glActiveTexture(GL_TEXTURE0);
	TEXTURE_MANAGER->BindTexture1D("RandomTexture");

	glBindVertexArray(updateVAO);
	glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[currentVBO]);

	//TODO: Make a function
	glEnableVertexAttribArray(TYPE);
	glVertexAttribIPointer(TYPE, 1, GL_INT, sizeof(Firework), 0);
	glEnableVertexAttribArray(POSITION);
	glVertexAttribPointer(POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Firework), (GLvoid*)sizeof(GLint));
	glEnableVertexAttribArray(VELOCITY);
	glVertexAttribPointer(VELOCITY, 3, GL_FLOAT, GL_FALSE, sizeof(Firework), (GLvoid*)(sizeof(GLint) + sizeof(glm::vec3)));
	glEnableVertexAttribArray(LIFETIME);
	glVertexAttribPointer(LIFETIME, 1, GL_FLOAT, GL_FALSE, sizeof(Firework), (GLvoid*)(sizeof(GLint) + 2 * sizeof(glm::vec3)));
	glEnableVertexAttribArray(COLOUR);
	glVertexAttribPointer(COLOUR, 4, GL_FLOAT, GL_FALSE, sizeof(Firework), (GLvoid*)(2 * sizeof(GLint) + 2 * sizeof(glm::vec3)));
	glEnableVertexAttribArray(FIREWORK_SIZE);
	glVertexAttribPointer(FIREWORK_SIZE, 1, GL_FLOAT, GL_FALSE, sizeof(Firework), (GLvoid*)(2 * sizeof(GLint) + 2 * sizeof(glm::vec3) + sizeof(glm::vec4)));
	
	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transfromFeedBackBuffer[currentFBO]);
	glBeginTransformFeedback(GL_POINTS);
	//  Set up buffer attributes //TODO: Do we need to do this every frame?

	

	// Next draw call will write to the TF buffer so 
	// don't want to spend time doing rasterisation
	

	// Redirect output to transform feedback buffer
	
	if (isFirst) { // Handle first draw call explicitly
		glDrawArrays(GL_POINTS, 0, 1);
		isFirst = false;
	}
	else {
		glDrawTransformFeedback(GL_POINTS, transfromFeedBackBuffer[currentVBO]);
	}

	// End of transform feedback
	glEndTransformFeedback();
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glDisable(GL_RASTERIZER_DISCARD);

	unsigned int count;
	glGetQueryObjectuiv(query, GL_QUERY_RESULT, &count); 
	glDeleteQueries(1, &query);
}


void FireworkSystem::RenderParticles(const glm::mat4& viewProj, const glm::vec3 cameraPos) {
	glDisable(GL_DEPTH_TEST);
	SHADER_MANAGER->SetShader(billboardShader);
	SHADER_MANAGER->SetUniform(billboardShader, "cameraPos", cameraPos);
	SHADER_MANAGER->SetUniform(billboardShader, "viewProjMatrix", viewProj);
	
	glActiveTexture(GL_TEXTURE0);
	TEXTURE_MANAGER->BindTexture("Firework");
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
	glEnable(GL_BLEND);

	// Data is in the transform feedback buffer at currentFBO which is attached to the vertex buffer particleBuffer[currentFBO] 
	// Note: Only a single attribute (position) is needed for rendering
	glBindVertexArray(billboardVAO);
	glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[currentFBO]);
	glEnableVertexAttribArray(0); // Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Firework), (GLvoid*)sizeof(GLfloat));
	glEnableVertexAttribArray(1); // Colour
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Firework), (GLvoid*)(2 * sizeof(GLint) + 2 * sizeof(glm::vec3)));
	glEnableVertexAttribArray(2); // Size
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Firework), (GLvoid*)(2 * sizeof(GLint) + 2 * sizeof(glm::vec3) + sizeof(glm::vec4)));
	glDrawTransformFeedback(GL_POINTS, transfromFeedBackBuffer[currentFBO]);
	glEnable(GL_DEPTH_TEST);
}

// As we are using transform feedback and need to specify the buffers before linking we will not use the shader manager to load our shader
void FireworkSystem::LoadShaders() {
	updateShader = "FireworkShader";
	Shader* shader = new Shader(SHADERDIR"FireworksVertex.glsl", SHADERDIR"FireworksFrag.glsl", SHADERDIR"FireworksGeom.glsl");

	// Specify the attributes that will be written into the transform feedback buffer
	const GLchar* varyings[NUM_ATTRIBUTES];
	varyings[TYPE] = "outType";
	varyings[POSITION] = "outPosition";
	varyings[VELOCITY] = "outVelocity";
	varyings[LIFETIME] = "outLifetime";
	varyings[COLOUR] = "outColour";
	varyings[FIREWORK_SIZE] = "outSize";
	glTransformFeedbackVaryings(shader->GetProgram(), NUM_ATTRIBUTES, varyings, GL_INTERLEAVED_ATTRIBS);

	if (shader->LinkProgram()) {
		SHADER_MANAGER->AddShader(updateShader, shader);
	}
	else {
		return;
	}
	SHADER_MANAGER->SetUniform(updateShader, "randomTex", 0);
	SHADER_MANAGER->SetUniform(updateShader, "launcherLifetime", 500.0f);
	SHADER_MANAGER->SetUniform(updateShader, "shellLifetime", 4000.0f);
	SHADER_MANAGER->SetUniform(updateShader, "secondaryShellLifetime", 2000.0f);

	//TODO: Move this to normal load	
	billboardShader = "BillboardShader";
	SHADER_MANAGER->AddShader(billboardShader, SHADERDIR"BillboardVertex.glsl", SHADERDIR"BillboardFrag.glsl", SHADERDIR"BillboardGeom.glsl");
	SHADER_MANAGER->SetUniform(updateShader, "diffuseTex", 0);
}