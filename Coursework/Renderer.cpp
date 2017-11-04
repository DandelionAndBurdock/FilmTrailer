#include "Renderer.h"

#include <glm/gtc/matrix_transform.hpp> // Projection matrix

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	heightMap = new HeightMap(TEXTUREDIR"terrain.raw");
	camera = new Camera(glm::vec3(0.0f, 500.0f, 1000.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	currentShader = new Shader(SHADERDIR"TexturedColourVertex.glsl", SHADERDIR"TexturedColourFragment.glsl");
	if (!currentShader->LinkProgram()) {
		return;
	}

	mapTexture = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.jpg",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	if (!mapTexture) {
		return;
	}

	SetTextureRepeating(mapTexture, true);

	projMatrix = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 1.0f, 10000.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	init = true;
}

Renderer::~Renderer() {
	delete heightMap;
	delete camera;
}

void Renderer::UpdateScene(float msec) {
	camera->UpdateCamera(msec);
	viewMatrix = camera->BuildViewMatrix();
}

void Renderer::RenderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(currentShader->GetProgram());
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);
	glBindTexture(GL_TEXTURE_2D, mapTexture);
	heightMap->Draw();

	glUseProgram(0);
	SwapBuffers();
}