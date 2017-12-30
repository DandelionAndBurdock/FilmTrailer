#include "PostProcessor.h"

#include "ShaderManager.h"

#include "../glm/mat4x4.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "../glm/gtx/transform.hpp"
#include "Mesh.h"
#include "TextureManager.h"

PostProcessor::PostProcessor(GLuint screenWidth, GLuint screenHeight)
{
	texWidth = screenWidth;
	texHeight = screenHeight;
	Initialise();
	
	sceneQuad = Mesh::GenerateQuad();

	blurPasses = 3;

	contrastLevel = 3.0;

	finalProcessTex = sceneColourTex;
}


PostProcessor::~PostProcessor()
{
	glDeleteFramebuffers(1, &sceneFBO);
	glDeleteFramebuffers(1, &processFBO);
	// Delete textures-> have overwitten one of them
	delete sceneQuad;
}



void PostProcessor::Initialise() {
	InitialiseSceneFBO();
	InitialiseProcessFBO();
}

void PostProcessor::InitialiseSceneFBO() {


	glGenTextures(1, &sceneDepthTex);
	glBindTexture(GL_TEXTURE_2D, sceneDepthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, texWidth, texHeight,
		0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);


	glGenTextures(1, &sceneColourTex);
	glBindTexture(GL_TEXTURE_2D, sceneColourTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texWidth, texHeight, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glGenFramebuffers(1, &sceneFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, sceneDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
		sceneDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		sceneColourTex, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return;


}

void PostProcessor::InitialiseProcessFBO() {
	glGenFramebuffers(1, &processFBO);

	for (int i = 0; i < 2; ++i) {
		glGenTextures(1, &processColourTex[i]);
		glBindTexture(GL_TEXTURE_2D, processColourTex[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texWidth, texHeight, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

}

void PostProcessor::BindSceneFBO() {
	glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
} 

#include <iostream>
void PostProcessor::ProcessScene() {
	finalProcessTex = sceneColourTex;
	if (blurOn) {
		GaussianBlur(sceneColourTex);
	}
	if (shatterOn) {
		Shatter();
	}
	//Bloom(sceneColourTex);
	//Contrast(sceneColourTex);
	//GaussianBlur(sceneColourTex);
}

void PostProcessor::BindProcessedTexture(){
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, finalProcessTex);
}

void PostProcessor::GaussianBlur(GLuint startTexture) {
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	
	SHADER_MANAGER->SetUniform("BlurShader", "projMatrix", glm::ortho(-1, 1, -1, 1));
	SHADER_MANAGER->SetUniform("BlurShader", "viewMatrix", glm::mat4());
	SHADER_MANAGER->SetUniform("BlurShader", "modelMatrix", glm::mat4());
	SHADER_MANAGER->SetUniform("BlurShader", "pixelSize", glm::vec2(1.0f / texWidth, 1.0f / texHeight));
	SHADER_MANAGER->SetUniform("BlurShader", "diffuseTex", 0);

	glDisable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);

	for (int i = 0; i < blurPasses; ++i) {
			// First pass
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D, processColourTex[1], 0);
			if (i == 0) {
				glBindTexture(GL_TEXTURE_2D, startTexture);
			}
			else {
				glBindTexture(GL_TEXTURE_2D, processColourTex[0]);
			}
		
			SHADER_MANAGER->SetUniform("BlurShader", "isVertical", 0);

			sceneQuad->Draw();

			// Second pass
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D, processColourTex[0], 0);
			glBindTexture(GL_TEXTURE_2D, processColourTex[1]);
			SHADER_MANAGER->SetUniform("BlurShader", "isVertical", 1);

			sceneQuad->Draw();
		}
		finalProcessTex = processColourTex[0];
		//std::swap(finalProcessTex, processColourTex[1]);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glEnable(GL_DEPTH_TEST);
}


void PostProcessor::Contrast(GLuint startTexture) {
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);

	SHADER_MANAGER->SetUniform("ContrastShader", "projMatrix", glm::ortho(-1, 1, -1, 1));
	SHADER_MANAGER->SetUniform("ContrastShader", "viewMatrix", glm::mat4());
	SHADER_MANAGER->SetUniform("ContrastShader", "modelMatrix", glm::mat4());
	SHADER_MANAGER->SetUniform("ContrastShader", "pixelSize", glm::vec2(1.0f / texWidth, 1.0f / texHeight));
	SHADER_MANAGER->SetUniform("ContrastShader", "diffuseTex", 0);
	SHADER_MANAGER->SetUniform("ContrastShader", "contrast", contrastLevel);

	glDisable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, startTexture);

	if (processColourTex[0] == startTexture) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, processColourTex[1], 0);
		finalProcessTex = processColourTex[1];
	}
	else {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, processColourTex[0], 0);
		finalProcessTex = processColourTex[0];
	}
	

	sceneQuad->Draw();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);

}


void PostProcessor::Bloom(GLuint startTexture) {
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);

	SHADER_MANAGER->SetUniform("BloomShader", "projMatrix", glm::ortho(-1, 1, -1, 1));
	SHADER_MANAGER->SetUniform("BloomShader", "viewMatrix", glm::mat4());
	SHADER_MANAGER->SetUniform("BloomShader", "modelMatrix", glm::mat4());
	SHADER_MANAGER->SetUniform("BloomShader", "pixelSize", glm::vec2(1.0f / texWidth, 1.0f / texHeight));
	SHADER_MANAGER->SetUniform("BloomShader", "diffuseTex", 0);

	glDisable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, startTexture);

	if (processColourTex[0] == startTexture) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, processColourTex[1], 0);
		finalProcessTex = processColourTex[1];
	}
	else {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, processColourTex[0], 0);
		finalProcessTex = processColourTex[0];
	}
	sceneQuad->Draw();
	
	GaussianBlur(finalProcessTex);
	Contrast(finalProcessTex);

	// Combine blurred highlight image with the original scene
	Combine(sceneColourTex, finalProcessTex);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}


void PostProcessor::Combine(GLuint sceneTexture, GLuint highlightTexture) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	SHADER_MANAGER->SetUniform("CombineShader", "projMatrix", glm::ortho(-1, 1, -1, 1));
	SHADER_MANAGER->SetUniform("CombineShader", "viewMatrix", glm::mat4());
	SHADER_MANAGER->SetUniform("CombineShader", "modelMatrix", glm::mat4());
	SHADER_MANAGER->SetUniform("CombineShader", "pixelSize", glm::vec2(1.0f / texWidth, 1.0f / texHeight));
	SHADER_MANAGER->SetUniform("CombineShader", "originalSceneTex", 0);
	SHADER_MANAGER->SetUniform("CombineShader", "blurredHighlightTex", 1);

	sceneQuad->Draw();

	glEnable(GL_DEPTH_TEST);
}

void PostProcessor::Shatter() {
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, processColourTex[0], 0);
	finalProcessTex = processColourTex[0];

	SHADER_MANAGER->SetUniform("ShatterShader", "projMatrix", glm::ortho(-1, 1, -1, 1));
	SHADER_MANAGER->SetUniform("ShatterShader", "viewMatrix", glm::mat4());
	SHADER_MANAGER->SetUniform("ShatterShader", "modelMatrix", glm::mat4());
	SHADER_MANAGER->SetUniform("ShatterShader", "texMatrix", glm::translate(glm::vec3(0.04f, 0.04f, 0.0f)));
	SHADER_MANAGER->SetUniform("ShatterShader", "upperThreshold", 0.55f);
	SHADER_MANAGER->SetUniform("ShatterShader", "lowerThreshold", 0.45f);

	SHADER_MANAGER->SetUniform("ShatterShader", "originalScene", 0);
	SHADER_MANAGER->SetUniform("ShatterShader", "smash", 1);

	glDisable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sceneColourTex);
	glActiveTexture(GL_TEXTURE1);
	TEXTURE_MANAGER->BindTexture("Smash");
	sceneQuad->Draw();

	SHADER_MANAGER->SetUniform("ShatterShader", "texMatrix", glm::translate(glm::vec3(-0.02f, -0.02f, 0.0f)));
	SHADER_MANAGER->SetUniform("ShatterShader", "upperThreshold", 0.3f);
	SHADER_MANAGER->SetUniform("ShatterShader", "lowerThreshold", 0.2f);
	sceneQuad->Draw();

	SHADER_MANAGER->SetUniform("ShatterShader", "texMatrix", glm::translate(glm::vec3(-0.02f, 0.02f, 0.0f)));
	SHADER_MANAGER->SetUniform("ShatterShader", "upperThreshold", 0.15f);
	SHADER_MANAGER->SetUniform("ShatterShader", "lowerThreshold", 0.1f);
	sceneQuad->Draw();

	SHADER_MANAGER->SetUniform("ShatterShader", "texMatrix", glm::translate(glm::vec3(0.01f, 0.05f, 0.0f)));
	SHADER_MANAGER->SetUniform("ShatterShader", "upperThreshold", 0.4f);
	SHADER_MANAGER->SetUniform("ShatterShader", "lowerThreshold", 0.35f);
	sceneQuad->Draw();

	SHADER_MANAGER->SetUniform("ShatterShader", "texMatrix", glm::translate(glm::vec3(-0.03f, 0.07f, 0.0f)));
	SHADER_MANAGER->SetUniform("ShatterShader", "upperThreshold", 0.80f);
	SHADER_MANAGER->SetUniform("ShatterShader", "lowerThreshold", 0.75f);
	sceneQuad->Draw();

	SHADER_MANAGER->SetUniform("ShatterShader", "texMatrix", glm::translate(glm::vec3(0.015f, 0.07f, 0.0f)));
	SHADER_MANAGER->SetUniform("ShatterShader", "upperThreshold", 0.90f);
	SHADER_MANAGER->SetUniform("ShatterShader", "lowerThreshold", 0.85f);
	sceneQuad->Draw();

	SHADER_MANAGER->SetUniform("ShatterShader", "texMatrix", glm::translate(glm::vec3(0.01f, 0.15f, 0.0f)));
	SHADER_MANAGER->SetUniform("ShatterShader", "upperThreshold", 0.75f);
	SHADER_MANAGER->SetUniform("ShatterShader", "lowerThreshold", 0.70f);
	sceneQuad->Draw();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}