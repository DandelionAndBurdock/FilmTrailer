#pragma once

#include "../../nclgl/OGLRenderer.h"
#include "../../nclgl/Camera.h"
#include "../../nclgl/SceneNode.h"
//#include "../../nclgl/Frustum.h"
#include "nclgl\CameraController.h"
#include "CubeRobot.h"

#include <algorithm>
#include <set>

class HeightMap;
class TextRenderer;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	virtual ~Renderer();

	virtual void UpdateScene(float msec);
	virtual void RenderScene();

protected:
	void UpdateUniforms();
	void UpdateMVP();

	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void ClearNodeLists();
	void DrawNodes();
	void DrawNode(SceneNode* n);

	void LoadShaders();
	void LoadTextures();
	void SetupCamera();
	void SetupScenes();
	void SetupSceneA();
	void ConfigureOpenGL();

	void DrawFPS();

	void DrawLine();
	GLuint VAOSample;
	void SetupLine();

	SceneNode* currentRoot;
	SceneNode* sceneARoot;
	CameraController* cameraControl;
	Camera* camera;
	Mesh* quad;


	//Frustum frameFrustum;
	// List of shaders used in the current frame
	std::set<std::string> activeShaders; 

	std::vector<SceneNode*> transparentNodeList;
	std::vector<SceneNode*> opaqueNodeList;

	GLuint quadTexture;

	HeightMap* terrain;

	GLuint mapTexture;

	TextRenderer* text;
};