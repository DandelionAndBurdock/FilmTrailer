// http://ogldev.atspace.co.uk/www/tutorial43/tutorial43.html
// https://learnopengl.com/#!Advanced-Lighting/Shadows/Point-Shadows
#pragma once

#include "../glm/vec3.hpp"

#include "../GLEW/include/GL/glew.h"

#include <string>

#include "../GLEW/include/GL/glew.h"

namespace Shadow {
	const int NUM_FACES = 6;			// Number of faces considered by the point light

										// Define the orientation of a camera for each face of the cube
	struct CameraDirection
	{
		GLenum cubeMapFace;
		glm::vec3 target;
		glm::vec3 up;
	};
	// Note: I think that the up vector is "non-conventional" because the texture coordinates are inverted 
	// Or is it actually because the shader uses the reverse incident vector?
	const CameraDirection cameraDirections[NUM_FACES] = {
		{ GL_TEXTURE_CUBE_MAP_POSITIVE_X, glm::vec3(+1.0f, +0.0f, +0.0f), glm::vec3(+0.0f, -1.0f, +0.0f) },
		{ GL_TEXTURE_CUBE_MAP_NEGATIVE_X, glm::vec3(-1.0f, +0.0f, +0.0f), glm::vec3(+0.0f, -1.0f, +0.0f) },
		{ GL_TEXTURE_CUBE_MAP_POSITIVE_Y, glm::vec3(+0.0f, +1.0f, +0.0f), glm::vec3(+0.0f, +0.0f, +1.0f) },
		{ GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, glm::vec3(+0.0f, -1.0f, +0.0f), glm::vec3(+0.0f, +0.0f, -1.0f) },
		{ GL_TEXTURE_CUBE_MAP_POSITIVE_Z, glm::vec3(+0.0f, +0.0f, +1.0f), glm::vec3(+0.0f, -1.0f, +0.0f) },
		{ GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, glm::vec3(+0.0f, +0.0f, -1.0f), glm::vec3(+0.0f, -1.0f, +0.0f) },
	};

	const GLfloat nearPlane = 1.0f;
	const GLfloat farPlane = 25.0f;
}

class Light;

class OmniShadow
{
public:
	OmniShadow(GLuint screenWidth, GLuint screenHeight);
	~OmniShadow();
	void BindForWriting();
	void BindForReading();
	void Unbind();
	// Creates transformation matrix for each direction of the light
	void SetUniforms(const Light* light);
protected:
	void Initialise();

	GLuint FBO; // FBO to render scene from light perspective
	GLuint shadowCubeMapID; // Handle to cube map representing all directions from point light // 
	GLuint depthTexID; // Handle to the depth buffer

	GLuint screenWidth;
	GLuint screenHeight;

	GLuint shadowTexWidth;
	GLuint shadowTexHeight;
	GLfloat aspect;

	std::string shader;
};

