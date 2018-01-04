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
class FireworkSystem;
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
class MD5Node;
class MD5FileData;
class SimpleShadow;
class ShaderArt;
class Scope;
class CameraController;

// Shader AI
class ShaderAI;
class Model;

class Renderer : public OGLRenderer {
	enum SceneNumber {SCENE_A, SCENE_B, SCENE_C, SCENE_D, SCENE_E, SCENE_F, NUM_SCENES};
	const float SCENE_TIME = 9000.0f; // Time in msec to display each scene 
	const float SCENE_TRANSITION_TIME = 2000.0f; // Time in msec to move from one scene to another
public:
	Renderer(Window& parent);
	virtual ~Renderer();

	virtual void UpdateScene(float msec);
	virtual void RenderScene();
	void RenderObjects(const glm::vec4& clipPlane);

protected:
	void PrepareToTransition();
	void Transition(SceneNumber from, SceneNumber to);
	void PostTransition();
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
	void SetupSceneD();
	void SetupSceneE();
	void SetupSceneF();
	void ConfigureOpenGL();
	void SetConstants();
	void HandleInput();

	void SetupReflectionBuffer();
	void SetupRefractionBuffer();
	// Helper Functions
	void RenderReflectionQuad();
	void RenderRefractionQuad();
	void RenderNoiseQuad();

	void RenderViewPointToBuffer(const glm::vec3& pos, const glm::vec3& lookat);
	void RenderSplitScreen(GLuint windowX, GLuint windowY, GLuint windowWidth, GLuint windowHeight);

	void ShadowMapFirstPass();
	void SimpleShadowFirstPass();
	void DrawSimpleShadowScene();

	void SceneSpecificUpdates(GLfloat msec);
	glm::vec3 ufoTargetPositions[2]; //Temp variables
	GLint ufoTargetIndex = 0;
	void SwitchUFOTargetPosition(); //Temporary function delete

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
	Mesh* skyboxQuad = nullptr;
	Mesh* sceneQuad = nullptr; // Holds scene for post processing
	Mesh* splitQuad = nullptr; // Holds images for multiple screens

	Frustum frameFrustum;
	// List of shaders used in the current frame
	std::set<std::string> activeShaders; 

	std::vector<SceneNode*> transparentNodeList;
	std::vector<SceneNode*> opaqueNodeList;

	GLuint quadTexture;//TODO: remove

	GLuint mapTexture;

	TextRenderer* text = nullptr;

	FireworkSystem* particleSystem = nullptr;
	ParticleManager* particleManager = nullptr;

	// List of all lights in the world
	std::vector<Light*> lights;
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
	SceneNode* laser = nullptr;

	// Modify ambient strenght of the scene to change mood
	GLfloat ambientStrength;

	OBJMesh* houseMesh = nullptr;
	SceneNode* houseNode = nullptr;

	OBJMesh* tree = nullptr;
	OBJMesh* ufoMesh = nullptr;
	SceneNode* ufoNode = nullptr;
	Sun* sun = nullptr;
	GLuint currentCubeMap;
	GLuint cubeMapA;
	GLuint cubeMapB;
	GLuint cubeMapC;
	GLuint cubeMapD;
	GLuint cubeMapE;
	GLuint cubeMapF;

	OmniShadow* omniShadow;

	// Disable Clip Plane doesn't work on all drives plus we still need to pass something to the function anyway
	const glm::vec4 NO_CLIP_PLANE = glm::vec4(0.0, 1.0, 0.0, std::numeric_limits<float>::max()); // A little bit hacky but this clip plane should result in no culling

	GLfloat nearPlane;
	GLfloat farPlane;

	GLfloat sceneTime = 0.0f;

	FlareManager* flareManager = nullptr;

	PostProcessor* postProcessor = nullptr;

	GLboolean pause = false;

	bool shadowRender = false;

	GLuint multipleViewBuffer;
	GLuint multipleViewTex;
	GLuint multipleViewDepthTex;
	Mesh* splitScreenMesh;
	 
	MD5Node* hellKnightNode;
	MD5FileData* hellKnightData;

	SimpleShadow* simpleShadow;

	ShaderArt* shaderArt;
	ShaderArt* blood;
	Scope* scope;

	float bloodCountdown = 8000.0f;


	// ShaderAI 
	ShaderAI* shaderAI;
	std::vector<Model*> text3D;
	void TestDraw();
	OBJMesh* bulb;
	SceneNode* bulbNode;

	// Reel
	CameraController* controller;

};