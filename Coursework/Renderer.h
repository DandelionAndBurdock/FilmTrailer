#pragma once

#include "../../nclgl/OGLRenderer.h"
#include "../../nclgl/Camera.h"
#include "../../nclgl/SceneNode.h"
#include "../../nclgl/Frustum.h"
#include "nclgl\CameraController.h"
#include "CubeRobot.h"

#include <algorithm>
#include <set>

//TODO: Forward declarations
class HeightMap;
class TextRenderer;
class ParticleSystem;
class ParticleManager;
class Light;
class DirectionalLight;
class Lightning;
class Spotlight;
class Grass; //TODO: Move to heightmap
class Water;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	virtual ~Renderer();

	virtual void UpdateScene(float msec);
	virtual void RenderScene();
	void RenderObjects();

protected:
	void UpdateUniforms();
	void UpdateLightUniforms(const std::string& shader, std::string uniform);

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

	SceneNode* currentRoot;
	SceneNode* sceneARoot;
	CameraController* cameraControl;
	Camera* camera;
	Mesh* quad;


	Frustum frameFrustum;
	// List of shaders used in the current frame
	std::set<std::string> activeShaders; 

	std::vector<SceneNode*> transparentNodeList;
	std::vector<SceneNode*> opaqueNodeList;

	GLuint quadTexture;

	HeightMap* terrain;

	GLuint mapTexture;

	TextRenderer* text;

	ParticleSystem* particleSystem;
	ParticleManager* particleManager;

	// List of lights in the world
	std::vector<Light*> lights;
	DirectionalLight* dirLight;

	Lightning* lightning;
	Spotlight* spotlight;
	Grass* grass;

	Water* water;
};