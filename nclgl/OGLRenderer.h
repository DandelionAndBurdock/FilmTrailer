#pragma once
/*
Class:OGLRenderer
Author:Rich Davison	<richard.davison4@newcastle.ac.uk>
Description:Abstract base class for the graphics tutorials. Creates an OpenGL 
3.2 CORE PROFILE rendering context. Each lesson will create a renderer that 
inherits from this class - so all context creation is handled automatically,
but students still get to see HOW such a context is created.

-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*/
#include "Common.h"

#include <string>
#include <fstream>
#include <vector>

#include "GL/glew.h"
#include "GL/wglew.h"

#include "SOIL.h"


#include "../glm/vec3.hpp"
#include "../glm/mat4x4.hpp"

#include "Window.h"


#include "Shader.h"		//Students make this file...
#include "Mesh.h"		//And this one...

using std::vector;


#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "SOIL.lib")


class Shader;

class OGLRenderer	{
public:
	friend class Window;
	OGLRenderer(Window &parent);
	virtual ~OGLRenderer(void);

	virtual void	RenderScene()		= 0;
	virtual void	UpdateScene(float msec);
	void			SwapBuffers();

	bool			HasInitialised() const;	

	glm::vec2 GetScreenSize();

protected:
	// Calculate an average of the current frames per second rounded to the nearest integer
	void CalculateFPS(float msec);

	virtual void	Resize(int x, int y);	
	
	int framesPerSecond;		  // Number of frames renderered to the screen per second
	const int NUM_FPS_FRAMES = 120; // Number of frames to average the FPS over
	std::vector<int> fps;		  // Holds values of the fps over the last 5 frames

	glm::mat4 projMatrix;		//Projection matrix
	glm::mat4 modelMatrix;	//Model matrix. NOT MODELVIEW
	glm::mat4 viewMatrix;		//View matrix
	glm::mat4 textureMatrix;	//Texture matrix

	glm::vec2 screenSize;

	int		width;			//Render area width (not quite the same as window width)
	int		height;			//Render area height (not quite the same as window height)
	bool	init;			//Did the renderer initialise properly?

	HDC		deviceContext;	//...Device context?
	HGLRC	renderContext;	//Permanent Rendering Context

	GLfloat time; //Time in msec since start
};