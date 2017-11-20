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

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	virtual ~Renderer();

	virtual void UpdateScene(float msec);
	virtual void RenderScene();
	void RenderObjects(const glm::vec4& clipPlane);

protected:
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
	void ConfigureOpenGL();

	void SetupReflectionBuffer();
	void SetupRefractionBuffer();
	// Helper Functions
	void RenderReflectionQuad();
	void RenderRefractionQuad();

	void ShadowMapFirstPass();

	void DrawFPS();

	SceneNode* currentRoot;
	SceneNode* sceneARoot;
	CameraController* cameraControl;
	Camera* camera;
	Mesh* reflectionQuad;
	Mesh* refractionQuad;
	Mesh* quad;


	Frustum frameFrustum;
	// List of shaders used in the current frame
	std::set<std::string> activeShaders; 

	std::vector<SceneNode*> transparentNodeList;
	std::vector<SceneNode*> opaqueNodeList;

	GLuint quadTexture;//TODO: remove

	HeightMap* terrain;

	GLuint mapTexture;

	TextRenderer* text;

	ParticleSystem* particleSystem;
	ParticleManager* particleManager;

	// List of all lights in the world
	std::vector<Light*> lights;
	// List of permanent lights in the world
	std::vector<Light*> permanentLights;
	// List of temporary lights in the world e.g. lightning
	std::vector<Light*> tempLights;

	DirectionalLight* dirLight;

	Lightning* lightning;
	Spotlight* spotlight;
	Grass* grass;

	Water* water;
	SceneNode* waterNode;

	// Modify ambient strenght of the scene to change mood
	GLfloat ambientStrength;

	OBJMesh* tree; 
	Sun* sun;
	GLuint cubeMap;

	OmniShadow* omniShadow;

	// Disable Clip Plane doesn't work on all drives plus we still need to pass something to the function anyway
	const glm::vec4 NO_CLIP_PLANE = glm::vec4(0.0, 1.0, 0.0, std::numeric_limits<float>::max()); // A little bit hacky but this clip plane should result in no culling

	GLfloat nearPlane;
	GLfloat farPlane;

	FlareManager* flareManager;
};