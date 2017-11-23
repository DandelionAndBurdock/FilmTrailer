#include "OmniShadow.h"

#include "ShaderManager.h"
#include "../glm/gtx/transform.hpp"

#include "Light.h"


OmniShadow::OmniShadow(GLuint screenWidth, GLuint screenHeight)
{
	this->screenWidth = screenWidth;
	this->screenHeight = screenHeight;

	shadowTexWidth = screenWidth;
	shadowTexHeight = screenHeight;
	aspect = shadowTexWidth / shadowTexHeight;

	shader = "ShadowDepth";
	SHADER_MANAGER->SetUniform("ShadowDepth", "farPlane", Shadow::farPlane);
	Initialise();
}


OmniShadow::~OmniShadow()
{
	glDeleteFramebuffers(1, &FBO);
	glDeleteTextures(1, &shadowCubeMapID);
	glDeleteTextures(1, &depthTexID);
}

void OmniShadow::Initialise() {
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	// Create depth buffer
	

	// Create cube map
	glGenTextures(1, &shadowCubeMapID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubeMapID);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
	// Setup each face of the cube map (GL_TEXTURE_CUBE_MAP_POSITIVE_X is the first enum)
	for (GLuint i = 0; i < Shadow::NUM_FACES; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32, shadowTexWidth, shadowTexWidth, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowCubeMapID, 0);

	
	// During the first shadow pass only the depth information is rendered so disable reads and writes to the colour buffer
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		__debugbreak();
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	
}



void OmniShadow::BindForWriting() {
	glViewport(0, 0, shadowTexWidth, shadowTexWidth);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	
}
void OmniShadow::BindForReading() {
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubeMapID);
}

void OmniShadow::Unbind() {
	glViewport(0, 0, screenWidth, screenHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OmniShadow::SetUniforms(const Light* light) {
	using namespace Shadow;

	glm::vec3 lightPos = glm::vec3(light->GetPosition());
	// FOV = 90 degrees to fill entire face
	glm::mat4 shadowProjMatrix = glm::perspective(glm::radians(90.0f), aspect, nearPlane, farPlane);

	glm::mat4 lightTransforms[6];
	for (int i = 0; i < NUM_FACES; ++i) {
		lightTransforms[i] = shadowProjMatrix * glm::lookAt(lightPos, lightPos + cameraDirections[i].target, cameraDirections[i].up);
		SHADER_MANAGER->SetUniform(shader, "lightViewMatrices[" + std::to_string(i) + "]", lightTransforms[i]);
	}
	SHADER_MANAGER->SetUniform(shader, "lightWorldPos", lightPos);
	SHADER_MANAGER->SetUniform(shader, "modelMatrix", light->GetTransform());

}