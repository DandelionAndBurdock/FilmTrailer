#pragma once

#include "../../nclgl/OGLRenderer.h"
#include "../../nclgl/Camera.h"
#include "../../nclgl/SceneNode.h"
//#include "../../nclgl/Frustum.h"
#include "CubeRobot.h"

#include <algorithm>

class HeightMap;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	virtual ~Renderer();

	virtual void UpdateScene(float msec);
	virtual void RenderScene();

protected:
	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void ClearNodeLists();
	void DrawNodes();
	void DrawNode(SceneNode* n);

	SceneNode* root;
	Camera* camera;
	Mesh* quad;

	//Frustum frameFrustum;

	std::vector<SceneNode*> transparentNodeList;
	std::vector<SceneNode*> nodeList;

	GLuint quadTexture;

	HeightMap* terrain;

	GLuint mapTexture;
};