#include "Water.h"

#include "Window.h"

#include "TextureManager.h"

Water::Water(GLint screenWidth, GLint screenHeight)
{
	SetupQuad();
	SetupFramebuffers();

	displayWidth = screenWidth;
	displayHeight = screenHeight;
}


Water::~Water()
{
	glDeleteFramebuffers(1, &refractionFBO);
	glDeleteFramebuffers(1, &reflectionFBO);

	glDeleteTextures(1, &refractionColourTexFBO);
	glDeleteTextures(1, &reflectionColourTexFBO);

	glDeleteRenderbuffers(1, &refractionDepthTexFBO);
	glDeleteRenderbuffers(1, &reflectionDepthTexFBO);
}

//TODO: This is just Mesh::GenerateQuad()
void Water::SetupQuad() {
	const int VERTS_PER_QUAD = 4;

	type = GL_TRIANGLE_STRIP;
	numVertices = VERTS_PER_QUAD;

	vertices = new glm::vec3[numVertices];
	vertices[0] = glm::vec3(-1.0f, -1.0f, +0.0f);
	vertices[1] = glm::vec3(-1.0f, +1.0f, +0.0f);
	vertices[2] = glm::vec3(+1.0f, -1.0f, +0.0f);
	vertices[3] = glm::vec3(+1.0f, +1.0f, +0.0f);

	colours = new glm::vec4[numVertices];
	colours[0] = glm::vec4(+1.0f, +0.0f, +0.0f, +1.0f);
	colours[1] = glm::vec4(+0.0f, +1.0f, +0.0f, +1.0f);
	colours[2] = glm::vec4(+0.0f, +0.0f, +1.0f, +1.0f);
	colours[3] = glm::vec4(+0.0f, +1.0f, +0.0f, +1.0f);

	textureCoords = new glm::vec2[numVertices];
	textureCoords[0] = glm::vec2(0.0f, 1.0f);
	textureCoords[1] = glm::vec2(0.0f, 0.0f);
	textureCoords[2] = glm::vec2(1.0f, 1.0f);
	textureCoords[3] = glm::vec2(1.0f, 0.0f);

	normals = new glm::vec3[numVertices];
	tangents = new glm::vec3[numVertices];
	for (int i = 0; i < 4; ++i) {
		normals[i] = glm::vec3(0.0f, 0.0f, -1.0f);
		tangents[i] = glm::vec3(1.0f, 0.0f, 0.0f);
	}

	BufferData();
}

void Water::SetupFramebuffers() {
	SetupReflectionFrameBuffer();
	SetupRefractionFrameBuffer();


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//TODO: Refactor -> CreateDepthAttachment, CreateColourAttachment...
void Water::SetupReflectionFrameBuffer() {
	glGenFramebuffers(1, &reflectionFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glGenTextures(1, &reflectionColourTexFBO);
	TEXTURE_MANAGER->AddTexture("Reflection", reflectionColourTexFBO);
	glBindTexture(GL_TEXTURE_2D, reflectionColourTexFBO);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, REFLECTION_WIDTH, REFLECTION_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, reflectionColourTexFBO, 0);


	glGenTextures(1, &reflectionDepthTexFBO); 
	glBindTexture(GL_TEXTURE_2D, reflectionDepthTexFBO);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, REFLECTION_WIDTH, REFLECTION_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, reflectionDepthTexFBO, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		__debugbreak();
		return;
	}
}

void Water::SetupRefractionFrameBuffer() {
	glGenFramebuffers(1, &refractionFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, refractionFBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glGenTextures(1, &refractionColourTexFBO);
	TEXTURE_MANAGER->AddTexture("Refraction", refractionColourTexFBO);
	glGenTextures(1, &refractionDepthTexFBO);

	glBindTexture(GL_TEXTURE_2D, refractionColourTexFBO);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, REFRACTION_WIDTH, REFRACTION_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, refractionColourTexFBO, 0);

	glGenTextures(1, &refractionDepthTexFBO); 
	TEXTURE_MANAGER->AddTexture("DepthMap", refractionDepthTexFBO);
	glBindTexture(GL_TEXTURE_2D, refractionDepthTexFBO);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, REFRACTION_WIDTH, REFRACTION_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, refractionDepthTexFBO, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		__debugbreak();
		return;
	}

}

void Water::BindReflectionFramebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glViewport(0, 0, REFLECTION_WIDTH, REFLECTION_HEIGHT);
}

void Water::BindRefractionFramebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, refractionFBO);
	glViewport(0, 0, REFRACTION_WIDTH, REFRACTION_HEIGHT);
}

void Water::UnbindFramebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, displayWidth, displayHeight);
}