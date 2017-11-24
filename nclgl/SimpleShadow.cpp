#include "SimpleShadow.h"

const float SimpleShadow::biasValues[16] = {
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0,
	0.5, 0.5, 0.5, 1.0
};

SimpleShadow::SimpleShadow(GLuint screenWidth, GLuint screenHeight)
{
	this->screenWidth = screenWidth;
	this->screenHeight = screenHeight;

	glGenTextures(1, &shadowDepthTex);
	glBindTexture(GL_TEXTURE_2D, shadowDepthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_SIZE, SHADOW_SIZE,
		0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, 0);


	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, shadowDepthTex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	for (int row = 0; row < NUM_MAT_DIMENSIONS; ++row) {
		for (int col = 0; col < NUM_MAT_DIMENSIONS; ++col) {
			biasMatrix[col][row] = biasValues[row + col];
		}
	}
}


SimpleShadow::~SimpleShadow()
{
	glDeleteFramebuffers(1, &shadowFBO);
	glDeleteTextures(1, &shadowDepthTex);
}




void SimpleShadow::BindShadow() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);
	//
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
}

void SimpleShadow::UnbindShadow() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, screenWidth, screenHeight);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void SimpleShadow::BindForReading() {
	glActiveTexture(GL_TEXTURE2);//Will break if node has more than 3 texture units
	glBindTexture(GL_TEXTURE_2D, shadowDepthTex);
}

