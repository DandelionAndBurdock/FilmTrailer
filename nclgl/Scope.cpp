#include "Scope.h"


#include "../glm/gtc/constants.hpp"
#include "../glm/trigonometric.hpp"
#include "../glm/gtx/transform.hpp"

#include "Shader.h"

#include "Texture.h"

Scope::Scope(GLfloat sWidth, GLfloat sHeight)
{
	shader = new Shader(SHADERDIR"2DVertex.glsl", SHADERDIR"2DFrag.glsl");
	if (!shader->WasLoaded()) {
		__debugbreak();
	}
	else {
		shader->LinkProgram();
	}

	crossShader = new Shader(SHADERDIR"TexturedQuadVertex2D.glsl", SHADERDIR"TexturedQuadFragment.glsl");
	if (!crossShader->WasLoaded()) {
		__debugbreak();
	}
	else {
		crossShader->LinkProgram();
	}

	crossTexture = new Texture(TEXTUREDIR"crosshair.png");
	quad = Mesh::GenerateNullQuad();


	float aspectRatio = sWidth / sHeight;
	projMatrix = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f);

	// Allocate space for x and y coordinate at each point on the circumference plus the centre point
	circleVertices = new GLfloat[2 * (NUM_POINTS + 1)];

	// Allocate space for indices. For every point in the circle must create a triangle
	// with the centre point and adjacent point
	circleIndices = new GLuint[NUM_POINTS * 3];

	glGenVertexArrays(1, &circleVAO);
	glBindVertexArray(circleVAO);

	glGenBuffers(1, &vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, 2 * (NUM_POINTS + 1) * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Indices won't change
	for (int i = 0; i < NUM_POINTS; i++, numIndices += 3) {
		circleIndices[i * 3] = NUM_POINTS;					// Centre vertex
		circleIndices[i * 3 + 1] = i;						// Current vertex
		circleIndices[i * 3 + 2] = (i + 1) % NUM_POINTS;	// Next vertex
	}

	GLuint indexBufferID;
	glGenBuffers(1, &indexBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLuint), circleIndices, GL_STATIC_DRAW);

	// Clean up
	glBindVertexArray(0);

}


Scope::~Scope()
{
	delete circleVertices;
	delete crossTexture;
	glDeleteBuffers(1, &vertexBufferID);
	glDeleteVertexArrays(1, &circleVAO);
}

void Scope::DrawCircle() {
	if (time < SWITCH_ON_TIME) {
		return;
	}
	shader->Use();
	shader->SetUniform("projMatrix", projMatrix);
	glBindVertexArray(circleVAO);
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
}
void Scope::UpdateCircle(GLfloat deltaSec) {
	time += deltaSec;
	centre.x = glm::sin(time * glm::two_pi<float>() / 5000.0f) * 1.0f;
	centre.y = glm::cos(time * glm::two_pi<float>() / 5000.0f) * 0.8f;
	RebufferVertices();
}

void Scope::DrawCrossHair() {
	if (time < SWITCH_ON_TIME) {
		return;
	}
	crossShader->Use();
	glActiveTexture(GL_TEXTURE0);
	crossTexture->Bind();
	crossShader->SetUniform("projMatrix", projMatrix);
	quad->Draw();
}

void Scope::RebufferVertices() {
	// Circumference
	for (GLint i = 0; i < NUM_POINTS; ++i) {
		GLfloat angle = glm::two_pi<GLfloat>() * i / NUM_POINTS;
		circleVertices[i * 2] = centre.x + glm::cos(angle) * radius;
		circleVertices[i * 2 + 1] = centre.y+ glm::sin(angle) * radius;
	}

	// Centre point
	circleVertices[NUM_POINTS * 2] = centre.x;
	circleVertices[NUM_POINTS * 2 + 1] = centre.y;

	glBindVertexArray(circleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 2 * (NUM_POINTS + 1) * sizeof(GLfloat), circleVertices);

	// Clean up
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Cross-Hair
	glm::vec3 quadVerts[4];
	quadVerts[0] = glm::vec3(centre + glm::vec2(-radius, -radius), 0.0f);
	quadVerts[1] = glm::vec3(centre + glm::vec2(radius, radius), 0.0f);
	quadVerts[2] = glm::vec3(centre + glm::vec2(-radius, radius), 0.0f);
	quadVerts[3] = glm::vec3(centre + glm::vec2(radius, -radius), 0.0f);
	quad->BufferVertices(quadVerts);
}