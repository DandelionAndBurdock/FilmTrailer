#pragma once

#include "../../nclgl/OGLRenderer.h"
#include "../../nclgl/Camera.h"
#include "../../nclgl/SceneNode.h"
#include "../../nclgl/Frustum.h"
#include "nclgl\CameraController.h"
#include "CubeRobot.h"

#include <algorithm>
#include <set>
#include <limits> // For numeric_limits<float>::max()

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
class OmniShadow;
class FlareManager;
class Sun;
class PostProcessor;
class Scene;
class GerstnerWaves;

class Renderer : public OGLRenderer {
	enum SceneNumber {SCENE_A, SCENE_B, SCENE_C, NUM_SCENES};
public:
	Renderer(Window& parent);
	virtual ~Renderer();

	virtual void UpdateScene(float msec);
	virtual void RenderScene();
	void RenderObjects(const glm::vec4& clipPlane);

protected:
	void Transition(SceneNumber from, SceneNumber to);
	void DrawSkybox();

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
	void SetupSceneB();
	void SetupSceneC();
	void ConfigureOpenGL();
	void SetConstants();
	void HandleInput();

	void SetupReflectionBuffer();
	void SetupRefractionBuffer();
	// Helper Functions
	void RenderReflectionQuad();
	void RenderRefractionQuad();
	void RenderNoiseQuad();

	void ShadowMapFirstPass();

	void DrawFPS();

	void DrawSceneToBuffer();
	void ProcessScene();
	void PresentScene();

	SceneNode* masterRoot = nullptr;
	SceneNumber currentScene;
	std::vector<Scene*> scenes;
	CameraController* cameraControl = nullptr;
	Camera* camera = nullptr;
	Mesh* reflectionQuad = nullptr;
	Mesh* refractionQuad = nullptr;
	Mesh* quad = nullptr;
	Mesh* sceneQuad = nullptr; // Holds scene for post processing


	Frustum frameFrustum;
	// List of shaders used in the current frame
	std::set<std::string> activeShaders; 

	std::vector<SceneNode*> transparentNodeList;
	std::vector<SceneNode*> opaqueNodeList;

	GLuint quadTexture;//TODO: remove

	GLuint mapTexture;

	TextRenderer* text = nullptr;

	ParticleSystem* particleSystem = nullptr;
	ParticleManager* particleManager = nullptr;

	// List of all lights in the world
	std::vector<Light*> lights;
	// List of permanent lights in the world
	std::vector<Light*> permanentLights;
	// List of temporary lights in the world e.g. lightning
	std::vector<Light*> tempLights;

	DirectionalLight* dirLight = nullptr;

	Lightning* lightning = nullptr;
	Spotlight* spotlight = nullptr;
	Grass* grass = nullptr;

	Water* water = nullptr;
	SceneNode* waterNode = nullptr;
	GerstnerWaves* oceanMesh = nullptr;
	SceneNode* oceanNode = nullptr;

	// Modify ambient strenght of the scene to change mood
	GLfloat ambientStrength;

	OBJMesh* tree = nullptr;
	Sun* sun = nullptr;
	GLuint currentCubeMap;
	GLuint cubeMapA;
	GLuint cubeMapB;

	OmniShadow* omniShadow;

	// Disable Clip Plane doesn't work on all drives plus we still need to pass something to the function anyway
	const glm::vec4 NO_CLIP_PLANE = glm::vec4(0.0, 1.0, 0.0, std::numeric_limits<float>::max()); // A little bit hacky but this clip plane should result in no culling

	GLfloat nearPlane;
	GLfloat farPlane;

	FlareManager* flareManager = nullptr;

	PostProcessor* postProcessor = nullptr;


};