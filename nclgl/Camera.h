/******************************************************************************
Class:Camera
Implements:
Author:Rich Davison	<richard.davison4@newcastle.ac.uk>
Description:FPS-Style camera. Uses the mouse and keyboard from the Window
class to get movement values!
Modifications: Philip Jones 06/11/2017 Modified Camera class to move relative to the
current viewing direction
-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Window.h"
#include "../glm/mat4x4.hpp"
#include "../glm/vec3.hpp"

class Camera	{
public:
	//TODO: Define x-axis, y-axis, z-axis
	Camera(glm::vec3 position = glm::vec3(0.0f), 
		   glm::vec3 viewDir = glm::vec3(0.0f, 0.0f, -1.0f), 
		   glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f));
	~Camera(void){};

	void UpdateCamera(float msec = 10.0f);

	//Builds a view matrix for the current camera variables, suitable for sending straight
	//to a vertex shader (i.e it's already an 'inverse camera matrix').
	glm::mat4 BuildViewMatrix();

	//Gets position in world space
	glm::vec3 GetPosition() const { return position;}
	//Sets position in world space
	void	SetPosition(glm::vec3 val) { position = val;}

	//Gets position in world space
	glm::vec3 GetViewDirection() const { return viewDirection; }
	//Sets position in world space
	void	SetViewDirection(glm::vec3 val) { viewDirection = val; }

protected:
	void HandleMouseUpdates();  // Updates pitch, yaw, (roll) and view direction based on relative mouse movement since last called
	void HandleKeyboardUpdates(float msec); // Moves camera position in response to keyboard input

	float cameraMoveSpeed;			// Controls how fast the camera position changes
	float cameraRotateSpeed;		// Controls how fast the camera rotates
	glm::vec3 position;

	glm::vec3 up;				// Points vertically upwards from the camera
	glm::vec3 viewDirection;	// Direction camera is pointing
	glm::vec3 right;			// Direction orthogonal to the up vector and view direction
};