#include "Renderer.h"

#include "glm/gtx/transform.hpp"
#include "nclgl/HeightMap.h"
#include "nclgl/TextureManager.h"
#include "nclgl/ShaderManager.h"
#include "nclgl/TextRenderer.h"
#include "nclgl\PerlinNoise.h"
#include "nclgl\ParticleSystem.h"
#include "nclgl\ParticleManager.h"
#include "nclgl\Light.h"
#include "nclgl\DirectionalLight.h"
#include "nclgl\Lightning.h"
#include "nclgl\Spotlight.h"
#include "nclgl\Grass.h"
#include "nclgl\Water.h"
#include "nclgl\OmniShadow.h"
#include "nclgl\FlareManager.h"
#include "nclgl\Sun.h"
#include "nclgl\PostProcessor.h"
#include "nclgl\Scene.h"
#include "nclgl\GerstnerWaves.h"
#include "nclgl\ParticleEmitter.h"
#include "nclgl\MD5Node.h"
#include "nclgl\MD5FileData.h"
#include "nclgl\SimpleShadow.h"
#include "nclgl\ShaderArt.h"
#include "nclgl\Scope.h"
#include "../glm/glm.hpp"
#include "../glm/gtx/transform.hpp"

// Ugly ASSIMP
#include "nclgl\Model.h"
#include "nclgl\ShaderAI.h"
#include <glm/gtc/type_ptr.hpp>


#include <algorithm> // For min()
#include <iostream>
#include <iomanip> // setprecision()


// Can't get these stupid shadows working last try...
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
unsigned int depthMapFBO;
unsigned int depthCubemap;
float near_plane = 1.0f;
float far_plane = 250.0f;
glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
std::vector<GLuint> shadowMatrixLocations;
GLuint farPlaneLocation;
GLuint modelMatrixLocation;

ShaderAI* depthShader;

// Add some more global variables
std::string reel[] = { "One", "Two", "Three", "Four", "Five" };

const int TEXT_POS_X = 850;
const int TEXT_POS_Y = 600;

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	CubeRobot::CreateCube();
	Light::CreateLightMesh();
	reflectionQuad = Mesh::GenerateQuad();
	refractionQuad = Mesh::GenerateQuad();
	sceneQuad = Mesh::GenerateQuad();
	
	SetupCamera();
	LoadShaders();
	LoadTextures();
	SetupScenes();
	ConfigureOpenGL();
	SetConstants();

	text = new TextRenderer(width, height, FONTSDIR"arial.ttf", 30, glm::vec3(1.0f, 1.0f, 1.0f));

	PerlinNoise p;
	p.GenerateTexture("Noise");

	flareManager = new FlareManager();

	omniShadow = new OmniShadow(screenSize.x, screenSize.y);

	postProcessor = new PostProcessor(screenSize.x, screenSize.y);

	init = true;
	time = 0.0f;

	skyboxQuad = Mesh::GenerateQuad();

	glGenFramebuffers(1, &multipleViewBuffer);
	glGenTextures(1, &multipleViewTex);
	glBindTexture(GL_TEXTURE_2D, multipleViewTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, screenSize.x, screenSize.y, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glBindFramebuffer(GL_FRAMEBUFFER, multipleViewBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		multipleViewTex, 0);

	glGenTextures(1, &multipleViewDepthTex);
	glBindTexture(GL_TEXTURE_2D, multipleViewDepthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, screenSize.x, screenSize.y,
		0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);


	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, multipleViewDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
		multipleViewDepthTex, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return;

	ufoTargetPositions[0] = glm::vec3(1500.0f, 600.0f, 1500.0f);
	ufoTargetPositions[1] = glm::vec3(300.0f, 900.0f, 300.0f);

	splitScreenMesh = Mesh::GenerateQuad();

	simpleShadow = new SimpleShadow(screenSize.x, screenSize.y);


	// Can't get these stupid shadows working. Last try...
	glGenFramebuffers(1, &depthMapFBO);
	// create depth cubemap texture
	glGenTextures(1, &depthCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	depthShader = new ShaderAI(SHADERDIR"ShadowDepthVertex.glsl", SHADERDIR"ShadowDepthFrag.glsl", SHADERDIR"ShadowDepthGeom.glsl");
	depthShader->Use();
	for (int i = 0; i < 6; ++i) {
		shadowMatrixLocations.push_back(glGetUniformLocation(depthShader->Program, std::string("lightViewMatrices[" + std::to_string(i) + "]").c_str()));
	}
	//farPlaneLocation = glGetUniformLocation(depthShader->Program, "farPlane");

	modelMatrixLocation = glGetUniformLocation(depthShader->Program, "modelMatrix");

	//glUniform1f(farPlaneLocation, farPlane);
	// End stupid shadow stuff
	sceneTime = 0.0f;
	// Scene B
	std::vector<glm::vec3> wps = { glm::vec3(1200.0f, 100.0f, 1100.0f), glm::vec3(1200.0f, 100.0f, 1100.0f), glm::vec3(800.0f, 100.0f, 1150.0f), glm::vec3(1900.0f, 500.0f, 0.0f), glm::vec3(1900.0f, 500.0f, 0.0f), glm::vec3(1200.0f, 100.0f, 1100.0f),  glm::vec3(1200.0f, 100.0f, 1100.0f) };
	std::vector<glm::vec3> vps = { glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f),  glm::normalize(glm::vec3(-1.0f, -1.0f, 1.0f)), glm::normalize(glm::vec3(-1.0f, -1.0f, 1.0f)),  glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f) };
	std::vector<float> tps = { 2500.0f, 5000.0f, 3000.0f,  5000.0f, 1000.0f, 9000.0f };

	// Scene C

	wps.push_back(glm::vec3(1200.0f, 800.0f, 1100.0f));
	vps.push_back(glm::vec3(glm::normalize(glm::vec3(1.0f, -1.0f, 0.0f))));
	tps.push_back(4000.0f);


	wps.push_back(glm::vec3(338.0f, 889.0f, 1093.0f));
	vps.push_back(glm::vec3(glm::normalize(glm::vec3(-0.65f, -0.06f, -0.75f))));
	tps.push_back(4000.0f);
	wps.push_back(glm::vec3(338.0f, 889.0f, 1093.0f));
	vps.push_back(glm::vec3(glm::normalize(glm::vec3(0.63f, -0.48f, 0.62f))));
	tps.push_back(4000.0f);

	wps.push_back(glm::vec3(1849.0f, 157.0f, 2560.0f));
	vps.push_back(glm::vec3(glm::normalize(glm::vec3(-0.37f, -0.23f, -0.898f))));
	tps.push_back(4000.0f);
	wps.push_back(glm::vec3(1849.0f, 157.0f, 2560.0f));
	vps.push_back(glm::vec3(glm::vec3(glm::normalize(glm::vec3(-0.37f, -0.23f, -0.898f)))));
	tps.push_back(7000.0f);

	wps.push_back(glm::vec3(1849.0f, 907.0f, 2560.0f));
	vps.push_back(glm::vec3(glm::vec3(glm::normalize(glm::vec3(-0.37f, -0.4f, -0.898f)))));
	tps.push_back(5000.0f);

	// Scene D
	wps.push_back(glm::vec3(636.0f, 808.0f, 1190.0f));
	vps.push_back(glm::vec3(glm::normalize(glm::vec3(0.0f, -0.6f, -0.8f))));
	tps.push_back(6000.0f);
	wps.push_back(glm::vec3(636.0f, 808.0f, 1190.0f));
	vps.push_back(glm::vec3(glm::normalize(glm::vec3(0.0f, -0.6f, -0.8f))));
	tps.push_back(7000.0f);
	wps.push_back(glm::vec3(546.0f, 51.0f, 1067.0f));
	vps.push_back(glm::vec3(glm::normalize(glm::vec3(0.8f, -0.0f, -0.4f))));
	tps.push_back(4000.0f);
	wps.push_back(glm::vec3(546.0f, 51.0f, 1067.0f));
	vps.push_back(glm::vec3(glm::normalize(glm::vec3(0.8f, -0.0f, -0.4f))));
	tps.push_back(40500.0f);
	//wps.push_back(glm::vec3(1836.0f, 208.0f, 800.0f));
	//vps.push_back(glm::vec3(glm::normalize(glm::vec3(-0.3f, -0.0f, -0.8f))));
	//tps.push_back(2500.0f);
	//wps.push_back(glm::vec3(1836.0f, 208.0f, 800.0f));
	//vps.push_back(glm::vec3(glm::normalize(glm::vec3(-0.3f, -0.0f, -0.8f))));
	//tps.push_back(12500.0f);
	controller = new CameraController(camera, wps, vps, tps); //Hippo
}; //Temp variables
	

//TODO: Check
Renderer::~Renderer() {
	//Functions delete meshes...
	if (dirLight)
		delete dirLight;
	if (spotlight)
		delete spotlight;
	delete camera;
	delete cameraControl;
//	delete particleSystem;
	delete particleManager;
	delete grass;
	delete refractionQuad;
	delete reflectionQuad;
	delete sun;
	delete sceneQuad;
	CubeRobot::DeleteCube();

	if (shaderArt) {
		delete shaderArt;
	}
	if (blood) {
		delete blood;
	}
	if (scope) {
		delete scope;
	}
	//Light::DeleteLightMesh();
}


void Renderer::SetupSceneA() {
	scenes.push_back(new Scene(masterRoot));
	shaderArt = new ShaderArt(screenSize.x, screenSize.y, SHADERDIR"PassThroughVertex.glsl", SHADERDIR"LavaLamp.frag");
	blood = new ShaderArt(screenSize.x, screenSize.y, SHADERDIR"PassThroughVertex.glsl", SHADERDIR"BloodFragment.glsl");
	scope = new Scope(screenSize.x, screenSize.y);
	scenes[SCENE_A]->SetCubeMap(cubeMapA);


	hellKnightData = new MD5FileData(MESHDIR"hellknight.md5mesh");
	hellKnightNode = new MD5Node(*hellKnightData);

	hellKnightData->AddAnim(MESHDIR"walk7.md5anim");
	hellKnightData->AddAnim(MESHDIR"idle2.md5anim");
	hellKnightData->AddAnim(MESHDIR"attack2.md5anim");
	hellKnightNode->PlayAnim(MESHDIR"walk7.md5anim");

	scenes[SCENE_A]->GetRoot()->AddChild(hellKnightNode);
	hellKnightNode->SetModelScale(glm::vec3(100.0f));
	return;//Revert
	SceneNode* heightMap = new SceneNode(new HeightMap, "TerrainShadowShader");
	

	scenes[SCENE_A]->SetTerrain(heightMap);
	heightMap->UseTexture("Terrain");
	heightMap->UseTexture("TerrainBump");

	scenes[SCENE_A]->AddLight(new Light(glm::vec3(500.0f, 200.0f, 500.0f), glm::vec4(1.0f), 2000.0f));

	if (dirLight) {
		delete dirLight;
	}

	sun = new Sun(glm::vec3(0.5f, -1.0f, 0.0f), glm::vec3(1.0, 1.0, 1.0), 1000.0f);
	dirLight = sun->GetLight();
	sun->SetTransform(glm::translate(sun->GetPosition()));
	sun->UseTexture("Sun");
	heightMap->AddChild(sun);

	if (spotlight) {
		delete spotlight;
	}


	lightning = new Lightning(glm::vec3(2000.0, 500.0, 2000.0), glm::vec3(2200.0, 0.0, 2200.0));//TODO: Double delete
	scenes[SCENE_A]->SetLightning(lightning);
	
	particleManager = new ParticleManager();
	scenes[SCENE_A]->SetParticles(particleManager);
	//Load meshes function
	OBJMesh* m = new OBJMesh; 
	if (m->LoadOBJMesh(MESHDIR"tree_oak.obj")) {
		tree = m;
	}
	else {
		__debugbreak();
	}
	OBJMesh* box = new OBJMesh;//TODO: Memory leak
	if (box->LoadOBJMesh(MESHDIR"Fixedcube.obj")) {
		SceneNode* cube = new SceneNode(box, "TerrainShader");
		cube->SetTransform(glm::translate(glm::vec3(2000.0, 0.0, 2000.0)));
		cube->SetModelScale(glm::vec3(100.0f));
		heightMap->AddChild(cube);
		cube->UseTexture("Flower");
		SceneNode* cube2 = new SceneNode(box, "TerrainShader");
		cube2->SetTransform(glm::translate(glm::vec3(2300.0, 0.0, 2000.0)));
		cube2->SetModelScale(glm::vec3(40.0f));
		heightMap->AddChild(cube2);
		cube2->UseTexture("Flower");
		SceneNode* cube3 = new SceneNode(box, "TerrainShader");
		cube3->SetTransform(glm::translate(glm::vec3(2000.0, 0.0, 2300.0)));
		cube3->SetModelScale(glm::vec3(60.0f));
		heightMap->AddChild(cube3);
		cube3->UseTexture("Flower");
	}
	else {
		__debugbreak();
	}


	
	//heightMap->AddChild(spotlight);
	
	SceneNode* t = new SceneNode(tree, "TerrainShader");
	t->SetModelScale(glm::vec3(50.0f));
	t->SetColour(glm::vec4(0.0f));// Add to transparent list
	t->SetTransform(glm::translate(glm::vec3(900.0f, 100.0f, 900.0f)));
	heightMap->AddChild(t);


	CubeRobot* cubey = new CubeRobot();
	cubey->SetTransform(glm::translate(glm::vec3(600.0f, 0.0f, 400.0f)) * glm::scale(glm::vec3(4.0f)));
	heightMap->AddChild(cubey);

	CubeRobot* cubey2 = new CubeRobot();
	cubey2->SetTransform(glm::translate(glm::vec3(600.0f, 0.0f, 800.0f)) * glm::scale(glm::vec3(4.0f)));
	heightMap->AddChild(cubey2);

	heightMap = nullptr;
}


void Renderer::HandleInput() {
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1)) {
		if (scenes.size() > 0) {
			Transition(currentScene, SCENE_A);
		}
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2)) {
		if (scenes.size() > 1) {
			Transition(currentScene, SCENE_B);
		}
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_3)) {
		if (scenes.size() > 2) {
			Transition(currentScene, SCENE_C);
		}
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_4)) {
		if (scenes.size() > 3) {
			Transition(currentScene, SCENE_D);
		}
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_5)) {
		if (scenes.size() > 4) {
			Transition(currentScene, SCENE_E);
		}
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_6)) {
		if (scenes.size() > 5) {
			Transition(currentScene, SCENE_F);
		}
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_P)) {
		pause = !pause;
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_LEFT)) {
		if (currentScene == 0) {
			Transition(currentScene, SceneNumber(SceneNumber::NUM_SCENES - 1));
		}
		else {
			Transition(currentScene, SceneNumber(currentScene - 1));
		}
		
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_RIGHT)) {
		Transition(currentScene, SceneNumber((currentScene + 1) % SceneNumber::NUM_SCENES));
	}
}

void Renderer::SetupSceneB() {
	scenes.push_back(new Scene(masterRoot));
	scenes[SCENE_B]->SetCubeMap(cubeMapB);
	SceneNode* heightMap = new SceneNode(new HeightMap(), "TerrainShadowShader");
	scenes[SCENE_B]->SetTerrain(heightMap);
	
	//Load meshes function
	//OBJMesh* m = new OBJMesh;
	//if (m->LoadOBJMesh(MESHDIR"Desp.obj")) {
	//	tree = m;
	//}
	//else {
	//	__debugbreak();
	//}
	//SceneNode* t = new SceneNode(tree, "TerrainShader");
	//t->SetModelScale(glm::vec3(50.0f));
	//heightMap->AddChild(t);

	OBJMesh* m = new OBJMesh;
	if (m->LoadOBJMesh(MESHDIR"sphere.obj")) {
		bulb = m;
	}
	else {
		__debugbreak();
	}
	bulbNode = new SceneNode(bulb, "LightShader");
	bulbNode->SetModelScale(glm::vec3(10.0f));
	heightMap->AddChild(bulbNode);
	heightMap->UseTexture("Terrain");
	heightMap->UseTexture("TerrainBump");

	scenes[SCENE_B]->AddLight(new Light(glm::vec3(500.0f, 1000.0f, 500.0f), glm::vec4(1.0f), 20000.0f));

	text3D.push_back(new Model(MESHDIR"/AssImp\\Philip.obj"));
	text3D.push_back(new Model(MESHDIR"/AssImp\\Jones.obj"));
	text3D.push_back(new Model(MESHDIR"/AssImp\\Graphics.obj"));
	shaderAI = new ShaderAI(SHADERDIR"VSTest.vs", SHADERDIR"FSTest.fs");

	return;

	//SceneNode* heightMap = new SceneNode(new HeightMap(TEXTUREDIR"terrain.raw"), "TerrainTexShader");
	//grass = new Grass(terrain, TEXTUREDIR"grassPack.png");
	scenes.push_back(new Scene(masterRoot));
	scenes[SCENE_B]->SetCubeMap(cubeMapB);
	scenes[SCENE_B]->SetTerrain(heightMap);
	heightMap->UseTexture("Terrain");
	heightMap->UseTexture("TerrainBump");
	heightMap->UseTexture("Sand");
	heightMap->UseTexture("SandGrass");
	heightMap->UseTexture("Grass");
	heightMap->UseTexture("Rock");

	


	ufoNode = new CubeRobot();
	ufoNode->SetTransform(glm::translate(glm::vec3(300.0f)));
	spotlight = new Spotlight(glm::vec3(0.0), glm::vec3(0.0, 0.0, 1.0), glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)));
	heightMap->AddChild(ufoNode);
	ufoNode->AddChild(spotlight);
	heightMap->AddChild(new CubeRobot());


}

void Renderer::SetupSceneC() {
	HeightMap* terrain = new HeightMap(TEXTUREDIR"terrain.raw");
	//	SceneNode* heightMap = new SceneNode(terrain, "TerrainShader");
	SceneNode* heightMap = new SceneNode(terrain, "TerrainMultiTexShader");
	
	scenes.push_back(new Scene(masterRoot));
	scenes[SCENE_C]->SetCubeMap(cubeMapC);
	scenes[SCENE_C]->SetTerrain(heightMap);
	heightMap->UseTexture("Terrain");
	heightMap->UseTexture("TerrainBump");
	heightMap->UseTexture("Sand");
	heightMap->UseTexture("SandGrass");
	heightMap->UseTexture("Grass");
	heightMap->UseTexture("Rock");


	sun = new Sun(glm::vec3(0.5f, -1.0f, 0.0f), glm::vec3(1.0, 1.0, 1.0), 1000.0f);
	dirLight = sun->GetLight();
	sun->SetTransform(glm::translate(sun->GetPosition()));
	sun->UseTexture("Sun");
	heightMap->AddChild(sun);

	//ufoNode = new CubeRobot();

	OBJMesh* m = new OBJMesh;
	if (!m->LoadOBJMesh(MESHDIR"sphere.obj")) {
		__debugbreak();
	}
	ufoNode = new SceneNode(m, "TerrainShader");
	ufoNode->UseTexture("Terrain");
	ufoNode->UseTexture("TerrainBump");
	ufoNode->SetModelScale(glm::vec3(100.0, 30.0f, 100.0));
	ufoNode->SetTransform(glm::translate(glm::vec3(300.0f)));
	spotlight = new Spotlight(glm::vec3(0.0), glm::vec3(0.0, 0.0, 1.0), glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)));
	spotlight->SetTransform(glm::translate(glm::vec3(0.0f, -30.0f, 0.0f)));
	heightMap->AddChild(ufoNode);
	ufoNode->AddChild(spotlight);

	water = new Water(GetScreenSize().x, GetScreenSize().y);
	waterNode = new SceneNode(water, "WaterShader");
	waterNode->SetTransform(glm::translate(glm::vec3(2000.0f, water->GetHeight(), 2000.0f)) * glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
	waterNode->SetModelScale(glm::vec3(1600.0f, 1600.0f, 1600.0f));
	waterNode->SetColour(glm::vec4(0.0f));// Add to transparent list
	waterNode->UseTexture("Reflection");
	waterNode->UseTexture("WaterBump");
	waterNode->UseTexture("Refraction");
	waterNode->UseTexture("dudvMap");
	waterNode->UseTexture("DepthMap");
	heightMap->AddChild(waterNode);

	cone= new OBJMesh;
	if (!cone->LoadOBJMesh(MESHDIR"cone.obj")) {
		__debugbreak();
	}
	coneNode = new SceneNode(cone, "ConeShader");
	coneNode->SetColour(glm::vec4(0.0f));
	coneNode->SetModelScale(glm::vec3(80.0f));
	spotlight->AddChild(coneNode);
	ufoNode->SetBoundingRadius(10000.0f);
	coneNode->SetBoundingRadius(100000.0f);
	return;
	scenes.push_back(new Scene(masterRoot));
	scenes[SCENE_C]->SetCubeMap(cubeMapC);
//	HeightMap* terrain = new HeightMap(TEXTUREDIR"PoolMap.data");
//	SceneNode* heightMap = new SceneNode(terrain, "TerrainShader");
	scenes[SCENE_C]->SetTerrain(heightMap);
	heightMap->UseTexture("Terrain");
	heightMap->UseTexture("TerrainBump");




	//scenes[SCENE_C]->AddLight(new Light(glm::vec3(+500.0f, 500.0f, -500.0f), glm::vec4(1.0f), 2000.0f));
	//scenes[SCENE_C]->AddLight(new Light(glm::vec3(+1500.0f, 500.0f, 00.0f), glm::vec4(1.0f), 2000.0f));
	scenes[SCENE_C]->AddLight(new Light(glm::vec3(+2500.0f, 500.0f, -500.0f), glm::vec4(1.0f), 2000.0f));
	//scenes[SCENE_C]->AddLight(new Light(glm::vec3(+3500.0f, 500.0f, -500.0f), glm::vec4(1.0f), 2000.0f));

	//oceanMesh = new GerstnerWaves;
	//oceanNode = new SceneNode(oceanMesh, "GerstnerShader");
	//oceanNode->UseTexture("Ocean");
	//oceanNode->UseTexture("OceanNormal");
	//oceanNode->SetTransform(glm::translate(glm::vec3(0.0f, 200.0f, -3500.0f)));
	//heightMap->AddChild(oceanNode);

	CubeRobot* cubey = new CubeRobot();
	cubey->SetTransform(glm::translate(glm::vec3(1500.0f, 300.0f, 2800.0f)) * glm::scale(glm::vec3(2.0f)));
	cubey->UseTexture("Flower");
	heightMap->AddChild(cubey);

	grass = new Grass(terrain, TEXTUREDIR"grassPack.png");

	cubey = nullptr;
	heightMap = nullptr;

}

void Renderer::SetupSceneD() {
	scenes.push_back(new Scene(masterRoot));
	HeightMap* terrain = new HeightMap();
	grass = new Grass(terrain, TEXTUREDIR"grassPack.png");
	SceneNode* heightMap = new SceneNode(new HeightMap(), "HoleTerrainShader");
	heightMap->UseTexture("Terrain");
	heightMap->UseTexture("TerrainBump");
	laser = new SceneNode(Mesh::GenerateQuad(), "TextureQuadShader");
	heightMap->AddChild(ufoNode);
	heightMap->AddChild(laser);
	laser->SetModelScale(glm::vec3(10.0f, 500.0f, 1.0f));
	laser->SetTransform(glm::translate(glm::vec3(500.0f, 0.0f, 500.0f)));
	SHADER_MANAGER->SetUniform("HoleTerrainShader", "epicentre", glm::vec2(500.0f, 500.0f));
	laser->UseTexture("Laser");
	scenes[SCENE_D]->SetTerrain(heightMap);
	scenes[SCENE_D]->SetCubeMap(cubeMapC);
	scenes[SCENE_D]->SetEmitter(new ParticleEmitter);
	
	lightning = new Lightning(glm::vec3(950.0f, 500.0f, 700.0f), glm::vec3(1150.0f, 0.0f, 800.0f));//TODO: Double delete
	scenes[SCENE_D]->SetLightning(lightning);


	return;
	//scenes.push_back(new Scene(masterRoot));
	//scenes[SCENE_D]->SetCubeMap(cubeMapD);
	//SceneNode* heightMap = new SceneNode(new HeightMap(), "CausticShader");
	//scenes[SCENE_D]->SetTerrain(heightMap);
	//scenes[SCENE_D]->AddLight(new Light(glm::vec3(+0.0f, 0.0f, 0.0f), glm::vec4(1.0f), 5000.0f));
	//heightMap->UseTexture("Brick");
	//
	//heightMap = nullptr;
}

void Renderer::SetupSceneE() {
	scenes.push_back(new Scene(masterRoot));
	scenes[SCENE_E]->SetFireworks(new FireworkSystem(glm::vec3(0.0f, 0.0f, 0.0f)));
	scenes[SCENE_E]->SetCubeMap(cubeMapF);
	return;
	scenes.push_back(new Scene(masterRoot));
	SceneNode* heightMap = new SceneNode(new HeightMap(), "HoleTerrainShader");
	heightMap->UseTexture("Terrain");
	heightMap->UseTexture("TerrainBump");
	laser = new SceneNode(Mesh::GenerateQuad(), "TextureQuadShader");
	heightMap->AddChild(laser);
	laser->SetModelScale(glm::vec3(10.0f, 500.0f, 1.0f));
	laser->SetTransform(glm::translate(glm::vec3(500.0f, 0.0f, 500.0f)));
	SHADER_MANAGER->SetUniform("HoleTerrainShader", "epicentre", glm::vec2(500.0f, 500.0f));
	laser->UseTexture("Laser");


	hellKnightData = new MD5FileData(MESHDIR"hellknight.md5mesh");
	hellKnightNode = new MD5Node(*hellKnightData);

	hellKnightData->AddAnim(MESHDIR"walk7.md5anim");
	hellKnightData->AddAnim(MESHDIR"idle2.md5anim");
	hellKnightNode->PlayAnim(MESHDIR"walk7.md5anim");


	heightMap->AddChild(hellKnightNode);
	hellKnightNode->SetModelScale(glm::vec3(100.0f));
	scenes[SCENE_E]->SetTerrain(heightMap);
	scenes[SCENE_E]->SetCubeMap(cubeMapC);
	scenes[SCENE_E]->SetEmitter(new ParticleEmitter);
	heightMap = nullptr;
}


void Renderer::SetupSceneF() {
	scenes.push_back(new Scene(masterRoot));
	return;
	//scenes.push_back(new Scene(masterRoot));
	//scenes[SCENE_F]->SetFireworks(new FireworkSystem(glm::vec3(0.0f, 0.0f, 0.0f)));
	//scenes[SCENE_F]->SetCubeMap(cubeMapF);
}

void Renderer::UpdateScene(float msec) {
	if (currentScene == SCENE_F && sceneTime > 6000) {
			Transition(SCENE_F, SCENE_A);
	}

	OGLRenderer::UpdateScene(msec);

	CalculateFPS(msec);

	HandleInput();

	projMatrix = glm::perspective(glm::radians(45.0f), (float)width / (float)height, nearPlane, farPlane);

	if (currentScene != SCENE_A && currentScene != SCENE_F) {
		//camera->UpdateCamera(msec);
		if (currentScene == SCENE_B && scope->GetRadius() < 3.0f) {
			
		}
		else {
			controller->Update(msec);
		}
		
	}
	
	viewMatrix = camera->BuildViewMatrix();



	if (!pause) {
		sceneTime += msec;
		if (masterRoot) {
			frameFrustum.FromMatrix(projMatrix * viewMatrix);
			BuildNodeLists(masterRoot);
			SortNodeLists();
			masterRoot->Update(msec);
		}

		if (currentScene == SCENE_C) {
			PrepareToTransition();
		}
		if (currentScene == SCENE_D) {
			coneNode->SetInactive();
			PostTransition();
		}
		

		scenes[currentScene]->UpdateEffects(msec, camera->GetPosition());

		textureMatrix = glm::rotate(glm::radians(time), glm::vec3(0.0, 1.0, 1.0));

		lights.clear();
		auto temp = scenes[currentScene]->GetLights();
		lights.insert(lights.end(), tempLights.begin(), tempLights.end());
		lights.insert(lights.end(), scenes[currentScene]->GetLights().begin(), scenes[currentScene]->GetLights().end());

		std::sort(lights.begin(), lights.end(), [](const Light* a, const Light* b) { return *a < *b; });

		SceneSpecificUpdates(msec);
	}



	UpdateUniforms();

	//particleSystem->UpdateParticles(msec);
	//particleManager->Update(msec, camera->GetPosition());
	//cameraControl->Update(msec);
 //TODO: Move camera construction to cameraControl
	
	//dirLight->Rotate(1.0 / 10.0 * msec, glm::vec3(0.0, 0.0, 1.0));


	

}

void Renderer::RenderObjects(const glm::vec4& clipPlane) {
	for (const auto& shader : activeShaders) {
		SHADER_MANAGER->SetUniform(shader, "clipPlane", clipPlane);
		SHADER_MANAGER->SetUniform(shader, "viewMatrix", camera->BuildViewMatrix()); //TODO: Store
	}
	DrawNodes();
	if (currentScene == SCENE_D) {
		grass->Draw(); 
	}
	

	scenes[currentScene]->DrawEffects(projMatrix * viewMatrix, camera->GetPosition());


}
void Renderer::RenderScene() {

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (currentScene == NUM_SCENES) {
		return;
	}

	if (currentScene == SCENE_A) {
		// Enable writing to the stencil buffer
		glEnable(GL_STENCIL_TEST);
		// Render circle updating contents of the stencil buffer
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF); // each bit ends up as 0 in the stencil buffer (disabling writes)
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDisable(GL_DEPTH_TEST);
		scope->DrawCircle();

		// Disable writing to the stencil buffer
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glStencilMask(0x00); // each bit ends up as 0 in the stencil buffer (disabling writes)

		glStencilFunc(GL_EQUAL, 1, 0xFF);
		DrawSkybox();

		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		shaderArt->Draw();
		blood->Draw();

		glStencilFunc(GL_EQUAL, 1, 0xFF);
		RenderObjects(NO_CLIP_PLANE);

		glStencilFunc(GL_EQUAL, 1, 0xFF);
		scope->DrawCrossHair();

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glStencilMask(~0);
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilMask(0x00);
		glDepthMask(GL_TRUE);
		if (sceneTime > 1000.0f && sceneTime < 5500.0f) {
			glDisable(GL_DEPTH_TEST);
			text->RenderText(std::string("Stencil Buffer"), TEXT_POS_X, TEXT_POS_Y, 1.5f);
			glEnable(GL_DEPTH_TEST);
		}
		SwapBuffers();
		glUseProgram(0);
		ClearNodeLists();
		activeShaders.clear();
		return;
	}
	else if (currentScene == SCENE_B) {
		//DrawSkybox();
		//RenderObjects(NO_CLIP_PLANE);
		//if (false){
		if (scope->GetRadius() < 2.99f) {
			// Enable writing to the stencil buffer
			glEnable(GL_STENCIL_TEST);
			// Render circle updating contents of the stencil buffer
			glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
			glStencilFunc(GL_ALWAYS, 1, 0xFF);
			glStencilMask(0xFF); // each bit ends up as 0 in the stencil buffer (disabling writes)
								 //glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			glDisable(GL_DEPTH_TEST);
			scope->DrawCircle();

			// Disable writing to the stencil buffer
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			glStencilMask(0x00); // each bit ends up as 0 in the stencil buffer (disabling writes)

			glStencilFunc(GL_EQUAL, 1, 0xFF);
			DrawSkybox();

			glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
			shaderArt->Draw();
			blood->Draw();

			glStencilFunc(GL_EQUAL, 1, 0xFF);
			RenderObjects(NO_CLIP_PLANE);
		

			glEnable(GL_DEPTH_TEST);
			TestDraw();
			glDisable(GL_STENCIL_TEST);
			glStencilMask(~0);
			glClear(GL_STENCIL_BUFFER_BIT);
			glStencilMask(0x00);
			glDepthMask(GL_TRUE);
			SwapBuffers();
			glUseProgram(0);
			ClearNodeLists();
			activeShaders.clear();
			return;
		}
		else {
		
			// Calculate Shadows
			if (!lights.empty()) {
				glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
				glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
				glEnable(GL_DEPTH_TEST);
				glClear(GL_DEPTH_BUFFER_BIT);
				//SHADER_MANAGER->SetShader("ShadowDepth");
				//TestDraw();
				depthShader->Use();
				glCullFace(GL_FRONT);
				for (int i = 0; i < 2; ++i) {
					glm::mat4 modelMat = glm::translate(glm::vec3((i + 5) * 200.0f, 100.0f * (1.5 - i), 60.0f * (i + 5))); // translate it down so it's at the center of the scene
					modelMat = modelMat * glm::scale(glm::vec3(200.0f, 200.0f, 200.0f));	// it's a bit too big for our scene, so scale it down
					glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMat));
					text3D[i]->Draw(*depthShader);
				}

				glm::mat4 modelMat = glm::translate(glm::vec3((8.2) * 200.0f, 100.0f * (0.5), 60.0f * (18))); // translate it down so it's at the center of the scene

				modelMat = modelMat * glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				modelMat = modelMat * glm::scale(glm::vec3(200.0f, 200.0f, 200.0f));	// it's a bit too big for our scene, so scale it down
				glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMat));
				text3D[2]->Draw(*depthShader);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glCullFace(GL_BACK);

				//omniShadow->BindForWriting();
				//omniShadow->SetUniforms(lights[0]);
				//shadowRender = true;
				//TestDraw();
				//shadowRender = false;
				//omniShadow->Unbind();
				//omniShadow->BindForReading();
			}
			// Draw Scene
			glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);


			postProcessor->BindSceneFBO();		
			DrawSkybox();
			TestDraw();
			
			if (sceneTime > 17000) {
				scenes[SCENE_B]->GetRoot()->SetInactive();
				scenes[SCENE_C]->GetRoot()->SetActive();
				currentCubeMap = scenes[SCENE_C]->GetCubeMap();
				currentScene = SCENE_C;
				BuildNodeLists(masterRoot);
				SortNodeLists();
				masterRoot->Update(0.0f);
				if (waterNode && waterNode->IsActive()) {
					glEnable(GL_CLIP_DISTANCE0);
					SetupReflectionBuffer();
					SetupRefractionBuffer();
					glDisable(GL_CLIP_DISTANCE0);
				}
				postProcessor->BindSpecialFBO();
				DrawSkybox();
				RenderObjects(NO_CLIP_PLANE);
				scenes[SCENE_C]->GetRoot()->SetInactive();
				scenes[SCENE_B]->GetRoot()->SetActive();
				UpdateUniforms();

				currentCubeMap = scenes[SCENE_B]->GetCubeMap();
				currentScene = SCENE_B;
				postProcessor->ShatterOn();
				glDisable(GL_BLEND);
				postProcessor->ProcessScene();
				postProcessor->ShatterOff();


			}
			else {

			}
	
			if (sceneTime > 8500.0f && sceneTime < 15000.0f) {
				glDisable(GL_DEPTH_TEST);
				text->RenderText(std::string("Multi-Directional Shadows!?!"), TEXT_POS_X, TEXT_POS_Y, 1.5f);
				glEnable(GL_DEPTH_TEST);
			}
			PresentScene();
			if (sceneTime > 21000.0f && sceneTime < 23000.0f) {
				glDisable(GL_DEPTH_TEST);
				text->RenderText(std::string("Textured Quad Water Effect"), TEXT_POS_X, TEXT_POS_Y, 1.5f);
				glEnable(GL_DEPTH_TEST);
			}
			DrawFPS();
			SwapBuffers();
			glUseProgram(0);
			ClearNodeLists();
			activeShaders.clear();
			return;
		}
	}
	else if (currentScene == SCENE_C) {
		if (waterNode && waterNode->IsActive()) {
			glEnable(GL_CLIP_DISTANCE0);
			SetupReflectionBuffer();
			SetupRefractionBuffer();
			glDisable(GL_CLIP_DISTANCE0);
		}
		DrawSceneToBuffer();
		postProcessor->ProcessScene();
		if (sceneTime > 25000) {
			coneNode->SetInactive();
			RenderViewPointToBuffer(ufoNode->GetPosition() - glm::vec3(0.0f, 50.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
			coneNode->SetActive();
		}
		glDisable(GL_BLEND);
		PresentScene();
		if (sceneTime > 25000) {
			RenderSplitScreen(100, 100, 150, 150);
		}

		flareManager->Render();
		DrawFPS();

		if (sceneTime > 4000.0f && sceneTime < 6000.0f) {
			glDisable(GL_DEPTH_TEST);
			text->RenderText(std::string("\"Lens Flare\""), TEXT_POS_X, TEXT_POS_Y, 1.5f);
			glEnable(GL_DEPTH_TEST);
		}
		else if (sceneTime > 25000) {
			glDisable(GL_DEPTH_TEST);
			text->RenderText(std::string("UFO Viewpoint"), TEXT_POS_X, TEXT_POS_Y, 1.5f);
			glEnable(GL_DEPTH_TEST);
		}

		SwapBuffers();
		glUseProgram(0);
		ClearNodeLists();
		activeShaders.clear();
		return;
	}
	else if (currentScene == SCENE_D) {
		DrawSceneToBuffer();
		postProcessor->ProcessScene();
		PresentScene();

		DrawFPS();
		if (sceneTime > 00.0f && sceneTime < 8000.0f) {
			glDisable(GL_DEPTH_TEST);
			text->RenderText(std::string("Dynamic Terrain"), TEXT_POS_X, TEXT_POS_Y, 1.5f);
			glEnable(GL_DEPTH_TEST);
		}
		else if (sceneTime > 9000.0f && sceneTime < 15000.0f) {
			glDisable(GL_DEPTH_TEST);
			text->RenderText(std::string("Lightning \& Particle Effect"), TEXT_POS_X, TEXT_POS_Y, 1.5f);
			glEnable(GL_DEPTH_TEST);
		}
		SwapBuffers();
		glUseProgram(0);
		ClearNodeLists();
		activeShaders.clear();
		return;
	}
	else if (currentScene == SCENE_E) {
		SHADER_MANAGER->SetUniform("OldFilm","background", 0);
		glActiveTexture(GL_TEXTURE0);
		TEXTURE_MANAGER->BindTexture("Fin");
		SHADER_MANAGER->SetUniform("OldFilm", "time", time);
		SHADER_MANAGER->SetUniform("OldFilm", "projMatrix", glm::ortho(-1, 1, -1, 1));
		SHADER_MANAGER->SetUniform("OldFilm", "viewMatrix", glm::mat4());
		SHADER_MANAGER->SetUniform("OldFilm", "modelMatrix", glm::mat4());
		sceneQuad->Draw();
		scenes[currentScene]->DrawEffects(projMatrix * viewMatrix, camera->GetPosition());
		if (sceneTime > 00.0f && sceneTime < 4000.0f) {
			glDisable(GL_DEPTH_TEST);
			text->RenderText(std::string("Transform Feedback \"Fireworks\""), TEXT_POS_X, TEXT_POS_Y, 1.5f);
			glEnable(GL_DEPTH_TEST);
		}
		SwapBuffers();
		glUseProgram(0);
		ClearNodeLists();
		activeShaders.clear(); //Rhino
		return;
	}
	else if (currentScene == SCENE_F) {
		SHADER_MANAGER->SetUniform("OldFilm", "background", 0);
		glActiveTexture(GL_TEXTURE0);
		if (sceneTime < 1000) {
			TEXTURE_MANAGER->BindTexture(reel[4]);
		}
		else if (sceneTime < 2000) {
			TEXTURE_MANAGER->BindTexture(reel[3]);
		}
		else if (sceneTime < 3000) {
			TEXTURE_MANAGER->BindTexture(reel[2]);
		}
		else if (sceneTime < 4000) {
			TEXTURE_MANAGER->BindTexture(reel[1]);
		}
		else if (sceneTime < 5000) {
			TEXTURE_MANAGER->BindTexture(reel[0]);
		}
			


		
		SHADER_MANAGER->SetUniform("OldFilm", "time", time);
		SHADER_MANAGER->SetUniform("OldFilm", "projMatrix", glm::ortho(-1, 1, -1, 1));
		SHADER_MANAGER->SetUniform("OldFilm", "viewMatrix", glm::mat4());
		SHADER_MANAGER->SetUniform("OldFilm", "modelMatrix", glm::mat4());
		sceneQuad->Draw();
		if (sceneTime > 2000 && sceneTime < 4000) {
			glDisable(GL_DEPTH_TEST);
			text->RenderText(std::string("Old Projector Effect"), TEXT_POS_X, TEXT_POS_Y, 1.5f);
			glEnable(GL_DEPTH_TEST);
		}
		SwapBuffers();
		glUseProgram(0);
		ClearNodeLists();
		activeShaders.clear();
		return;
	}

	//*************** WATER**************************
	// Enable clipping planes so that we don't have to process geometry
	// above/below the water for refraction/reflection
	if (waterNode && waterNode->IsActive()) {
		glEnable(GL_CLIP_DISTANCE0);
		SetupReflectionBuffer();
		SetupRefractionBuffer();
		glDisable(GL_CLIP_DISTANCE0);
	}
	//*************** SHADOW *****************************
	if (currentScene == SCENE_A) {
		ShadowMapFirstPass();
		omniShadow->BindForReading();
	}
	else {
		SimpleShadowFirstPass();
	}

	//*************RENDER SCENE **************************
	DrawSceneToBuffer();
	// Draw mini-scene for multiple views
	if (currentScene == SCENE_B) {
		RenderViewPointToBuffer(ufoNode->GetPosition(), glm::vec3(0.0f, -1.0f, 0.0f));
	}
	//*************POST PROCESSING **************************
	 postProcessor->ProcessScene();
	PresentScene();

	//*************RENDER GUI**************************
	//RenderReflectionQuad();
	//RenderRefractionQuad();
	//RenderNoiseQuad();
	if (currentScene == SCENE_B) {
		RenderSplitScreen(100, 100, 150, 150);
	}
	else if (currentScene == SCENE_F) {
		scenes[SCENE_F]->DrawEffects(projMatrix * viewMatrix, camera->GetPosition());
	}
	flareManager->Render();
	DrawFPS();

	SwapBuffers();
	glUseProgram(0);
	ClearNodeLists();
	activeShaders.clear();

}

void Renderer::DrawSceneToBuffer() {
	postProcessor->BindSceneFBO();
	DrawSkybox();
	RenderObjects(NO_CLIP_PLANE);
	scenes[currentScene]->DrawEffects(projMatrix * viewMatrix, camera->GetPosition());
}

//TODO: Draw Scene()
void Renderer::ProcessScene() {
	postProcessor->ProcessScene();
}

void Renderer::PresentScene() {
	postProcessor->BindProcessedTexture();
	SHADER_MANAGER->SetUniform("QuadShader", "projMatrix", glm::ortho(-1, 1, -1, 1));
	SHADER_MANAGER->SetUniform("QuadShader", "viewMatrix", glm::mat4());
	SHADER_MANAGER->SetUniform("QuadShader", "modelMatrix", glm::mat4());
	sceneQuad->Draw();
}

void Renderer::BuildNodeLists(SceneNode* from) {
	if (from->GetMesh() && frameFrustum.InsideFrustum(*from)) {
		glm::vec3 translation = glm::vec3(from->GetWorldTransform()[3]);
		glm::vec3 dir = translation - camera->GetPosition();

		from->SetCameraDistance(glm::dot(dir, dir));

			activeShaders.insert(from->GetShaderName());

		if (from->GetColour().w < 1.0f) {
			transparentNodeList.push_back(from);
		}
		else {
			opaqueNodeList.push_back(from);
		}
	}


	for (auto iter = from->GetChildIteratorStart(); iter != from->GetChildIteratorEnd(); ++iter) {
		if ((*iter)->IsActive()) {
			BuildNodeLists(*iter);
		}
	}

}

void Renderer::SortNodeLists() {
	std::sort(transparentNodeList.begin(), transparentNodeList.end(), SceneNode::CompareByCameraDistance);
	std::sort(opaqueNodeList.begin(), opaqueNodeList.end(), SceneNode::CompareByCameraDistance);
}


void Renderer::DrawNodes() {
	for (auto iter = opaqueNodeList.begin(); iter != opaqueNodeList.end(); ++iter) {
		DrawNode(*iter);
	}

	for (auto iter = transparentNodeList.rbegin(); iter != transparentNodeList.rend(); ++iter) {
		DrawNode(*iter);
	}
}


void Renderer::DrawNode(SceneNode* n) {
	if (n->GetMesh()) {
		if (shadowRender) {
			SHADER_MANAGER->SetUniform("ShadowDepth", "modelMatrix", n->GetWorldTransform() * glm::scale(n->GetModelScale()));
		}
		else {
			SHADER_MANAGER->SetUniform(n->GetShaderName(), "modelMatrix", n->GetWorldTransform() * glm::scale(n->GetModelScale()));
		}
		
		
		n->DrawNode();
	}
}




void Renderer::ClearNodeLists() {
	transparentNodeList.clear();
	opaqueNodeList.clear();

}

void Renderer::DrawFPS() {
	return;
	glDisable(GL_DEPTH_TEST);
	std::stringstream ss;
	//ss << std::fixed << std::setprecision(0) << framesPerSecond;
	ss << framesPerSecond;
	text->RenderText(std::string("FPS: ") + ss.str(), 100, 100, 1.0f);
	glEnable(GL_DEPTH_TEST);
}

void Renderer::SetupScenes() { //Warthog
	masterRoot = new SceneNode();
	SetupSceneA();
	currentScene = SCENE_F; // Rever
	currentCubeMap = cubeMapF;
	SetupSceneB();
	SetupSceneC();
	SetupSceneD();
	SetupSceneE();
	SetupSceneF();
	scenes[SCENE_C]->GetRoot()->SetInactive();
	ufoNode->SetActive();
	scenes[SCENE_B]->GetRoot()->SetInactive();
	scenes[SCENE_D]->GetRoot()->SetInactive();
	scenes[SCENE_A]->GetRoot()->SetInactive();

	return; //Revert
	SetupSceneD();
	SetupSceneE();
	SetupSceneF();
	currentCubeMap = cubeMapA;
	currentScene = SCENE_A;
	scenes[SCENE_B]->GetRoot()->SetInactive();
	scenes[SCENE_C]->GetRoot()->SetInactive();
	scenes[SCENE_D]->GetRoot()->SetInactive();
	scenes[SCENE_E]->GetRoot()->SetInactive();
//	scenes[SCENE_F]->GetRoot()->SetInactive();
}

void Renderer::LoadShaders() {
	SHADER_MANAGER->AddShader("TextShader", SHADERDIR"TextVertex.glsl", SHADERDIR"TextFragment.glsl");
	SHADER_MANAGER->AddShader("TerrainShadowShader", SHADERDIR"LightingVertex.glsl", SHADERDIR"OmniShadowFrag.glsl");
	SHADER_MANAGER->AddShader("TerrainShader", SHADERDIR"LightingVertex.glsl", SHADERDIR"LightingFragment.glsl");
	//SHADER_MANAGER->AddShader("TerrainShader", SHADERDIR"LightingHeightVertex.glsl", SHADERDIR"LightingFragment.glsl");

	Misisng:SHADER_MANAGER->AddShader("TerrainMultiTexShader", SHADERDIR"LightingVertexMultiTex.glsl", SHADERDIR"LightingFragmentMultiTex.glsl");
	SHADER_MANAGER->AddShader("HoleTerrainShader", SHADERDIR"DeformVertexDouble.txt", SHADERDIR"DeformFragment.glsl");
	SHADER_MANAGER->AddShader("ShadowDepth", SHADERDIR"ShadowCubeMapVertex.glsl", SHADERDIR"ShadowCubeMapFrag.glsl", SHADERDIR"ShadowCubeMapGeom.glsl");
	SHADER_MANAGER->AddShader("AnimShader", SHADERDIR"AnimVertexNCLGL.glsl", SHADERDIR"AnimFragmentNCLGL.glsl");
	// Missing: SHADER_MANAGER->AddShader("ExplodeAnimShader", SHADERDIR"AnimVertexNCLGL.glsl", SHADERDIR"AnimFragmentNCLGL.glsl", SHADERDIR"ExplodingGeom.glsl");
	// Missing: SHADER_MANAGER->AddShader("SimpleShadowDepth", SHADERDIR"ShadowVert.glsl", SHADERDIR"ShadowFrag.glsl");
	// Missing: SHADER_MANAGER->AddShader("SimpleShadowScene", SHADERDIR"ShadowSceneVert.glsl", SHADERDIR"ShadowSceneFrag.glsl");
	SHADER_MANAGER->AddShader("QuadShader", SHADERDIR"TexturedVertex.glsl", SHADERDIR"TexturedFragment.glsl");
	SHADER_MANAGER->AddShader("TextureQuadShader", SHADERDIR"TexturedQuadVertex.glsl", SHADERDIR"TexturedQuadFragment.glsl"); // Adding this one just so I don't break an
	SHADER_MANAGER->AddShader("OldFilm", SHADERDIR"NoiseVertex.glsl", SHADERDIR"NoiseFragment2.glsl");
	//SHADER_MANAGER->AddShader("OldFilm", SHADERDIR"NoiseVertex.glsl", SHADERDIR"NoiseFragment.glsl");
	SHADER_MANAGER->AddShader("LightShader", SHADERDIR"SceneVertex.glsl", SHADERDIR"SceneFragment.glsl");
	SHADER_MANAGER->AddShader("WaterShader", SHADERDIR"WaterVertex.glsl", SHADERDIR"WaterFragment.glsl");
	SHADER_MANAGER->AddShader("CubeMapShader", SHADERDIR"skyboxVertex.glsl", SHADERDIR"skyboxFragment.glsl");
	SHADER_MANAGER->AddShader("SunShader", SHADERDIR"SimpleBillBoardVertex.glsl", SHADERDIR"SimpleBillBoardFrag.glsl");
	SHADER_MANAGER->AddShader("FlareShader", SHADERDIR"FlareVertex.glsl", SHADERDIR"FlareFragment.glsl");
	SHADER_MANAGER->AddShader("GerstnerShader", SHADERDIR"GerstnerVertex.glsl", SHADERDIR"GerstnerFragment.glsl");
	// Missing: SHADER_MANAGER->AddShader("CausticShader", SHADERDIR"CausticVertex3JustCheat.glsl", SHADERDIR"CausticFragment3JustCheat.glsl");
	SHADER_MANAGER->AddShader("ParticleShader", SHADERDIR"vertex.glsl", SHADERDIR"fragment.glsl", SHADERDIR"geometry.glsl");
	SHADER_MANAGER->AddShader("ConeShader", SHADERDIR"SceneVertex.glsl", SHADERDIR"SceneColourFragment.glsl");
	// Post processing shaders
	SHADER_MANAGER->AddShader("BlurShader", SHADERDIR"TexturedVertex.glsl", SHADERDIR"BlurFragment.glsl");
	SHADER_MANAGER->AddShader("BloomShader", SHADERDIR"TexturedVertex.glsl", SHADERDIR"BloomFragment.glsl");
	SHADER_MANAGER->AddShader("ContrastShader", SHADERDIR"TexturedVertex.glsl", SHADERDIR"ContrastFragment.glsl");
	SHADER_MANAGER->AddShader("ShatterShader", SHADERDIR"TexturedTexVertex.glsl", SHADERDIR"ShatterFragment.glsl");
	// Missing: SHADER_MANAGER->AddShader("CombineShader", SHADERDIR"TexturedVertex.glsl", SHADERDIR"CombineFragment.glsl");
}

void Renderer::LoadTextures() {
	TEXTURE_MANAGER->AddTexture("StainedGlass", TEXTUREDIR"stainedglass.tga");
	TEXTURE_MANAGER->AddTexture("Terrain", TEXTUREDIR"Barren Reds.jpg");
	TEXTURE_MANAGER->AddTexture("TerrainBump", TEXTUREDIR"Barren RedsDOT3.jpg");
	TEXTURE_MANAGER->AddTexture("dudvMap", TEXTUREDIR"waterDUDV.png");
	TEXTURE_MANAGER->AddTexture("waterBump", TEXTUREDIR"waterNormalMap.png");
	TEXTURE_MANAGER->AddTexture("Sand", TEXTUREDIR"sand.png");
	TEXTURE_MANAGER->AddTexture("Grass", TEXTUREDIR"grass.png");
	TEXTURE_MANAGER->AddTexture("SandGrass", TEXTUREDIR"sandGrass.jpg");
	TEXTURE_MANAGER->AddTexture("Rock", TEXTUREDIR"rock.png");
	TEXTURE_MANAGER->AddTexture("Flower", TEXTUREDIR"Flower.png");
	TEXTURE_MANAGER->AddTexture("Ocean", TEXTUREDIR"Ocean.png");
	TEXTURE_MANAGER->AddTexture("OceanNormal", TEXTUREDIR"ocean_normal.png");
	TEXTURE_MANAGER->AddTexture("Brick", TEXTUREDIR"brick.tga");
	TEXTURE_MANAGER->AddTexture("Explosion", TEXTUREDIR"ParticleAtlas.png");
	TEXTURE_MANAGER->AddTexture("Laser", TEXTUREDIR"blueLaser.jpg");
	TEXTURE_MANAGER->AddTexture("Smash", TEXTUREDIR"TestSmashNoAANoLine.png");
	TEXTURE_MANAGER->AddTexture("Fin", TEXTUREDIR"Fin.png");
	TEXTURE_MANAGER->AddTexture(reel[0], TEXTUREDIR"1.png");
	TEXTURE_MANAGER->AddTexture(reel[1], TEXTUREDIR"2.png");
	TEXTURE_MANAGER->AddTexture(reel[2], TEXTUREDIR"3.png");
	TEXTURE_MANAGER->AddTexture(reel[3], TEXTUREDIR"4.png");
	TEXTURE_MANAGER->AddTexture(reel[4], TEXTUREDIR"5.png");

	TEXTURE_MANAGER->AddTexture("Sun", TEXTUREDIR"sun.png");
	for (int i = 1; i <= 8; ++i) {
		TEXTURE_MANAGER->AddTexture(std::string("Flare") + std::to_string(i), TEXTUREDIR + std::string("Flare") + std::to_string(i) + std::string(".png"));
	}
	
	cubeMapC = SOIL_load_OGL_cubemap(
		TEXTUREDIR"lake1_lf.jpg",
		TEXTUREDIR"lake1_rt.jpg",
		TEXTUREDIR"lake1_up.jpg",
		TEXTUREDIR"lake1_dn.jpg",
		TEXTUREDIR"lake1_ft.jpg",
		TEXTUREDIR"lake1_bk.jpg",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0
	);

	if (!cubeMapC) {
		std::cout << SOIL_last_result() << std::endl;
		__debugbreak();
	}

	cubeMapB = SOIL_load_OGL_cubemap(
		TEXTUREDIR"rusted_west.jpg",
		TEXTUREDIR"rusted_east.jpg",
		TEXTUREDIR"rusted_up.jpg",
		TEXTUREDIR"rusted_down.jpg",
		TEXTUREDIR"rusted_south.jpg",
		TEXTUREDIR"rusted_north.jpg",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0
	);

	if (!cubeMapB) {
		std::cout << SOIL_last_result() << std::endl;
		__debugbreak();
	}

	cubeMapA = SOIL_load_OGL_cubemap(
		TEXTUREDIR"land_lf.jpg",
		TEXTUREDIR"land_rt.jpg",
		TEXTUREDIR"land_up.jpg",
		TEXTUREDIR"land_dn.jpg",
		TEXTUREDIR"land_ft.jpg",
		TEXTUREDIR"land_bk.jpg",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0
	);

	if (!cubeMapA) {
		std::cout << SOIL_last_result() << std::endl;
		__debugbreak();
	}

	cubeMapD = SOIL_load_OGL_cubemap(
		TEXTUREDIR"awup_lf.tga",
		TEXTUREDIR"awup_rt.tga",
		TEXTUREDIR"awup_up.tga",
		TEXTUREDIR"awup_dn.tga",
		TEXTUREDIR"awup_bk.tga",
		TEXTUREDIR"awup_ft.tga",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0
	);

	if (!cubeMapD) {
		std::cout << SOIL_last_result() << std::endl;
		//Missing__debugbreak();
	}

	cubeMapF = SOIL_load_OGL_cubemap(
		TEXTUREDIR"purplenebula_lf.tga",
		TEXTUREDIR"purplenebula_rt.tga",
		TEXTUREDIR"purplenebula_up.tga",
		TEXTUREDIR"purplenebula_dn.tga",
		TEXTUREDIR"purplenebula_bk.tga",
		TEXTUREDIR"purplenebula_ft.tga",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0
	);

	if (!cubeMapF) {
		std::cout << SOIL_last_result() << std::endl;
		//Missing:__debugbreak();
	}

}

void Renderer::ConfigureOpenGL() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void Renderer::SetupCamera() {
	camera = new Camera();
	camera->SetPosition(glm::vec3(0.0f, 100.0f, 750.0f));
	std::vector<glm::vec3> wps = { glm::vec3(-2000.0f, 1800.0f, -2000.0f), glm::vec3(2000.0f, -1800.0f, -2000.0f) , glm::vec3(2000.0f, 1800.0f, 2000.0f), glm::vec3(-2000.0f, -1800.0f, 2000.0f) }; //TODO: Read in from file
	std::vector<glm::vec3> lps = { glm::vec3(00.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) , glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-0.0f, 0.0f, 0.0f) };
//	cameraControl = new CameraController(camera, wps, lps);
}


// This was an attempt to make setting uniforms nicer but spiralled into a horrible mess
void Renderer::UpdateUniforms() {

	activeShaders.insert("CubeMapShader");
	if (currentScene == SCENE_D) {
		activeShaders.insert("ParticleShader");
	}
	if (currentScene == SCENE_A) {
		// Revert: activeShaders.insert("Particle");
	}
	for (const auto& shader : activeShaders) {
		std::vector<std::string> uniforms = SHADER_MANAGER->GetUniformNames(shader);
		for (const auto& uniform : uniforms) {//TODO: Refactor
			if (uniform == "particleSize") {
				continue; // No time to handle this now
			}
			if (uniform == "epicentre") {
				continue; // No time to handle this now
			}
			if (uniform == "holeTime") {
				continue; // No time to handle this now
			}
			if (uniform == "weightTex") {
				continue; // No time to handle this now
			}
			if (uniform == "transformTex") {
				continue; // No time to handle this now
			}
			if (uniform.find("Tex") != std::string::npos) {
				if (uniform == "diffuseTex") {
					SHADER_MANAGER->SetUniform(shader, uniform, 0);
				}
				else if (uniform == "bumpTex") {
					SHADER_MANAGER->SetUniform(shader, uniform, 1);
				}
				else if (uniform == "reflectionTex") {
					SHADER_MANAGER->SetUniform(shader, uniform, 0);
				}
				else if (uniform == "refractionTex") {
					SHADER_MANAGER->SetUniform(shader, uniform, 2);
				}
				else if (uniform == "dudvMapTex") {
					SHADER_MANAGER->SetUniform(shader, uniform, 3);
				}
				else if (uniform == "depthMapTex") {
					SHADER_MANAGER->SetUniform(shader, uniform, 4);
				}
				else if (uniform == "heightMapTex") {
					SHADER_MANAGER->SetUniform(shader, uniform, 5);
				}
				else if (uniform == "cubeTex") {
					SHADER_MANAGER->SetUniform(shader, uniform, 0);
				}
				else if (uniform == "sandTex") {
					SHADER_MANAGER->SetUniform(shader, uniform, 2);
				}
				else if (uniform == "sandGrassTex") {
					SHADER_MANAGER->SetUniform(shader, uniform, 3);
				}
				else if (uniform == "grassTex") {
					SHADER_MANAGER->SetUniform(shader, uniform, 4);
				}
				else if (uniform == "rockTex") {
					SHADER_MANAGER->SetUniform(shader, uniform, 5);
				}
				else if (uniform == "shadowTex") {
					SHADER_MANAGER->SetUniform(shader, uniform, 3);
				}
				else {
					std::cout << "Warning: " << uniform << " was not set by renderer" << std::endl;
				}
			}
			else if (uniform.find("Light") != std::string::npos) {
				UpdateLightUniforms(shader, uniform);
			}
			else {
				if (uniform == "modelMatrix") {
					SHADER_MANAGER->SetUniform(shader, uniform, modelMatrix);
				}
				else if (uniform == "viewMatrix") {
					SHADER_MANAGER->SetUniform(shader, uniform, viewMatrix);
				}
				else if (uniform == "projMatrix") {
					SHADER_MANAGER->SetUniform(shader, uniform, projMatrix);
				}
				else if (uniform == "viewProjMatrix") {
					SHADER_MANAGER->SetUniform(shader, uniform, projMatrix * viewMatrix);
				}
				else if (uniform == "biasMatrix") {
					SHADER_MANAGER->SetUniform(shader, uniform, simpleShadow->GetBiasMatrix());
				}
				else if (uniform == "textureMatrix") {
					SHADER_MANAGER->SetUniform(shader, uniform, textureMatrix);
				}
				else if (uniform == "ambientStrength") {
					SHADER_MANAGER->SetUniform(shader, uniform, ambientStrength);
				}
				else if (uniform == "cameraPos") {
					SHADER_MANAGER->SetUniform(shader, uniform, camera->GetPosition());
				}
				else if (uniform == "cameraRight") {
					SHADER_MANAGER->SetUniform(shader, uniform, camera->GetRight());
				}
				else if (uniform == "cameraUp") {
					SHADER_MANAGER->SetUniform(shader, uniform, camera->GetUp());
				}
				else if (uniform == "time") {
					SHADER_MANAGER->SetUniform(shader, uniform, time / 1000.0f);
				}
				else if (uniform == "clipPlane") {
					// Do nothing will set on the fly
				}
				else if (uniform == "nearPlane") {
					SHADER_MANAGER->SetUniform(shader, uniform, nearPlane);
				}
				else if (uniform == "farPlane") {
					SHADER_MANAGER->SetUniform(shader, uniform, farPlane);
				}
				else if (uniform == "particleCentre") { // Should remove this one if time allows
					SHADER_MANAGER->SetUniform(shader, uniform, sun->GetPosition()); 
				}
				else {
					std::cout << "Warning: " << uniform << " was not set by renderer" << std::endl;
				}
			}
		}

	}
}


void Renderer::UpdateLightUniforms(const std::string& shader, std::string uniform) {
	if (uniform.find("directionalLights") != std::string::npos) {
		if (dirLight) {
			if (uniform == "directionalLights[0].direction") {
				SHADER_MANAGER->SetUniform(shader, uniform, dirLight->GetDirection());
			}
			else if (uniform == "directionalLights[0].colour") {
				SHADER_MANAGER->SetUniform(shader, uniform, dirLight->GetColour());
			}
			else {
				std::cout << "Warning: " << uniform << " was not set by renderer" << std::endl;
			}
		}
		else {
			if (uniform == "directionalLights[0].direction") {
				SHADER_MANAGER->SetUniform(shader, uniform, glm::vec4(-1.0f));
			}
			else if (uniform == "directionalLights[0].colour") {
				SHADER_MANAGER->SetUniform(shader, uniform, glm::vec4(-1.0f));
			}
			else {
				std::cout << "Warning: " << uniform << " was not set by renderer" << std::endl;
			}
		}
	
		 
	}
	else if (uniform.find("pointLights") != std::string::npos) {//TODO:Setting uniforms 9 times instead of 3!!
		// Number of lights to render
		int numLights = std::min(int(lights.size()), MAX_LIGHTS);
		for (int i = 0; i < numLights; ++i) {
			std::stringstream ss;
			ss << i;
			std::string s = ss.str();
			SHADER_MANAGER->SetUniform(shader, "pointLights[" + ss.str() + "].position", lights[i]->GetPosition());
			SHADER_MANAGER->SetUniform(shader, "pointLights[" + ss.str() + "].colour", lights[i]->GetColour());
			SHADER_MANAGER->SetUniform(shader, "pointLights[" + ss.str() + "].radius", lights[i]->GetRadius());
		}
		for (int i = numLights; i < MAX_LIGHTS; ++i) {
			std::stringstream ss;
			ss << i;
			std::string s = ss.str();
			SHADER_MANAGER->SetUniform(shader, "pointLights[" + ss.str() + "].position", glm::vec4(-1.0f));
			SHADER_MANAGER->SetUniform(shader, "pointLights[" + ss.str() + "].colour", glm::vec4(-1.0f));
			SHADER_MANAGER->SetUniform(shader, "pointLights[" + ss.str() + "].radius", -1.0f);
		}
		
	}
	else if (uniform.find("spotLights") != std::string::npos) {
		if (spotlight && spotlight->IsActive()) {
			if (uniform == "spotLights[0].direction") {
				SHADER_MANAGER->SetUniform(shader, uniform, spotlight->GetDirection());
			}
			else if (uniform == "spotLights[0].colour") {
				SHADER_MANAGER->SetUniform(shader, uniform, spotlight->GetColour());
			}
			else if (uniform == "spotLights[0].innerCutOff") {
				SHADER_MANAGER->SetUniform(shader, uniform, spotlight->GetInnerCutOff());
			}
			else if (uniform == "spotLights[0].outerCutOff") {
				SHADER_MANAGER->SetUniform(shader, uniform, spotlight->GetOuterCutOff());
			}
			else if (uniform == "spotLights[0].position") {
				SHADER_MANAGER->SetUniform(shader, uniform, spotlight->GetPosition());
			}
			else if (uniform == "spotLights[0].radius") {
				SHADER_MANAGER->SetUniform(shader, uniform, spotlight->GetRadius());
			}
			else {
				std::cout << "Warning: " << uniform << " was not set by renderer" << std::endl;
			}
		}
	}
	else {
		if (uniform == "spotLights[0].direction") {
			SHADER_MANAGER->SetUniform(shader, uniform, glm::vec4(-1.0f));
		}
		else if (uniform == "spotLights[0].colour") {
			SHADER_MANAGER->SetUniform(shader, uniform, glm::vec4(-1.0f));
		}
		else if (uniform == "spotLights[0].innerCutOff") {
			SHADER_MANAGER->SetUniform(shader, uniform, -1.0f);
		}
		else if (uniform == "spotLights[0].outerCutOff") {
			SHADER_MANAGER->SetUniform(shader, uniform, -1.0f);
		}
		else if (uniform == "spotLights[0].position") {
			SHADER_MANAGER->SetUniform(shader, uniform, glm::vec4(-1.0f));
		}
		else if (uniform == "spotLights[0].radius") {
			SHADER_MANAGER->SetUniform(shader, uniform, -1.0f);
		}
		else {
			std::cout << "Warning: " << uniform << " was not set by renderer" << std::endl;
		}
	}

}

void Renderer::ShadowMapFirstPass() {
	if (lights.empty()) {
		return;
	}
	omniShadow->BindForWriting();
	omniShadow->SetUniforms(lights[0]);
	shadowRender = true;
	RenderObjects(NO_CLIP_PLANE);
	shadowRender = false;
	omniShadow->Unbind();
}

void Renderer::DrawSkybox() {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, currentCubeMap);
	glDepthMask(GL_FALSE);
	SHADER_MANAGER->SetShader("CubeMapShader");
	skyboxQuad->Draw();
	glUseProgram(0);
	glDepthMask(GL_TRUE);
}


void Renderer::RenderReflectionQuad() {
	projMatrix = glm::ortho(-1, 1, -1, 1);
	viewMatrix = glm::mat4();
	glViewport(0, 0, 200, 200);
	SHADER_MANAGER->SetShader("QuadShader");
	SHADER_MANAGER->SetUniform("QuadShader", "modelMatrix", glm::mat4());
	SHADER_MANAGER->SetUniform("QuadShader", "viewMatrix", viewMatrix);
	SHADER_MANAGER->SetUniform("QuadShader", "projMatrix", projMatrix);

	SHADER_MANAGER->SetUniform("QuadShader", "diffuseTex", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, water->GetReflectionTex());
	reflectionQuad->Draw(); //TODO: Don't need two quads unless for naming

	// Reset view port
	glViewport(0, 0, screenSize.x, screenSize.y);
}
void Renderer::RenderRefractionQuad() {
	glViewport(300, 0, 200, 200);
	SHADER_MANAGER->SetShader("QuadShader");
	SHADER_MANAGER->SetUniform("QuadShader", "modelMatrix", glm::mat4());
	SHADER_MANAGER->SetUniform("QuadShader", "viewMatrix", viewMatrix);
	SHADER_MANAGER->SetUniform("QuadShader", "projMatrix", projMatrix);

	SHADER_MANAGER->SetUniform("QuadShader", "diffuseTex", 0);
	glBindTexture(GL_TEXTURE_2D, water->GetRefractionTex());
	reflectionQuad->Draw();


	// Reset view port
	glViewport(0, 0, screenSize.x, screenSize.y);
}

void Renderer::SetupReflectionBuffer() {
	water->BindReflectionFramebuffer();
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	// To get correct reflection sample we need to mirror (reflect) the camera position/pitch relative to the water plane
	float cameraMirrorShift = camera->GetPosition().y - water->GetHeight(); // Height of camera above the water plane
	camera->Reflect(cameraMirrorShift);
	SHADER_MANAGER->SetUniform("CubeMapShader", "viewMatrix", camera->BuildViewMatrix());
	DrawSkybox();
	waterNode->SetInactive();
	RenderObjects(glm::vec4(0.0, 1.0, 0.0, -water->GetHeight()));
	camera->Reflect(-cameraMirrorShift);
}

void Renderer::SetupRefractionBuffer() {
	SHADER_MANAGER->SetUniform("CubeMapShader", "viewMatrix", camera->BuildViewMatrix());
	water->BindRefractionFramebuffer();
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawSkybox();
	RenderObjects(glm::vec4(0.0, -1.0, 0.0, water->GetHeight()));
	water->UnbindFramebuffer(); //Binds Window FBO
	waterNode->SetActive();
}




void Renderer::Transition(SceneNumber from, SceneNumber to) {
	// Turn off current scene
	scenes[from]->GetRoot()->SetInactive();
	scenes[to]->GetRoot()->SetActive();
	currentCubeMap = scenes[to]->GetCubeMap();
	currentScene = to;
	sceneTime = 0.0f;

}

void Renderer::SetConstants() {
	ambientStrength = 0.2;
	nearPlane = 1.0f;
	farPlane = 5000.0f;
}

void Renderer::RenderNoiseQuad() {
	SHADER_MANAGER->SetShader("QuadShader");
	SHADER_MANAGER->SetUniform("QuadShader", "modelMatrix", glm::mat4());
	SHADER_MANAGER->SetUniform("QuadShader", "viewMatrix", glm::mat4());
	SHADER_MANAGER->SetUniform("QuadShader", "projMatrix", glm::ortho(-1, 1, -1, 1));

	SHADER_MANAGER->SetUniform("QuadShader", "diffuseTex", 0);
	TEXTURE_MANAGER->BindTexture("Noise");
	reflectionQuad->Draw();
}

void Renderer::RenderViewPointToBuffer(const glm::vec3& pos, const glm::vec3& viewDirection) {
	glBindFramebuffer(GL_FRAMEBUFFER, multipleViewBuffer);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); 
	glm::mat4 newViewMatrix = glm::lookAt(pos, pos + viewDirection, glm::vec3(1, 0, 0)); // Last element is up vector of camera need to pass in as parm but this will work for now
	frameFrustum.FromMatrix(projMatrix * newViewMatrix);
	BuildNodeLists(masterRoot);
	SortNodeLists();
	for (const auto& shader : activeShaders) {
		SHADER_MANAGER->SetUniform(shader, "clipPlane", NO_CLIP_PLANE);
		SHADER_MANAGER->SetUniform(shader, "viewMatrix", newViewMatrix); //TODO: Store
	}
	DrawSkybox();
	DrawNodes();
	
	scenes[currentScene]->DrawEffects(projMatrix * newViewMatrix, pos);
}

void Renderer::RenderSplitScreen(GLuint windowX, GLuint windowY, GLuint windowWidth, GLuint windowHeight) {
	glViewport(windowX, windowY, windowWidth, windowHeight);
	glDisable(GL_DEPTH_TEST);
	SHADER_MANAGER->SetUniform("QuadShader", "modelMatrix", glm::mat4());
	SHADER_MANAGER->SetUniform("QuadShader", "viewMatrix", glm::mat4());
	SHADER_MANAGER->SetUniform("QuadShader", "projMatrix", glm::ortho(-1, 1, -1, 1));

	SHADER_MANAGER->SetUniform("QuadShader", "diffuseTex", 0);
	glBindTexture(GL_TEXTURE_2D, multipleViewTex);
	splitScreenMesh->Draw();
	// Reset viewport
	glViewport(0, 0, screenSize.x, screenSize.y);
	glEnable(GL_DEPTH_TEST);
}

#include <glm/gtx/norm.hpp>
// Temporary ugly function
void Renderer::SceneSpecificUpdates(GLfloat msec) {
	std::cout << camera->GetPosition().x << "," << camera->GetPosition().y << "," << camera->GetPosition().z << std::endl;
	std::cout << camera->GetViewDirection().x << "," << camera->GetViewDirection().y << "," << camera->GetViewDirection().z << std::endl;
	if (currentScene == SCENE_A) {
		if (shaderArt) {
			if (scope->GetRadius() > 0.399f) {
				shaderArt->Update(msec / 1000.0f);
			}
			else {
				blood->Update(msec / 1000.0f);
				bloodCountdown -= msec;
			}
				
			scope->UpdateCircle(msec / 1000.0f);
			if (hellKnightNode->IsIdle()) {
				hellKnightNode->PlayAnim(MESHDIR"idle2.md5anim");
			}
	
		}
		if (bloodCountdown < 0.0f) {
			camera->SetPosition(glm::vec3(1200.0f, 100.0f, 1100.0f));
			Transition(SCENE_A, SCENE_B);
		}
	}

	if (currentScene == SCENE_B) {
		if (sceneTime > 27000.0f) {
			Transition(SCENE_B, SCENE_C);
		}
		float timeSec = msec / 1000.0f;
		
		if (scope->GetRadius() < 3.0f) {
			scope->IncreaseRadius(timeSec * timeSec * 200.0f);
			scope->RebufferVertices();
			sceneTime = 0.0f;
		}


		//lights[0]->SetPosition(glm::vec3(300.0f + 100.0f * sin(msec/ 100.0f), 300.0f + 100.0f * cos(msec / 100.0f), 100.0f + 100.0f  * sin(msec/ 100.0f)));
		if (!lights.empty()) {
			if (sceneTime < 4800) {
				lights[0]->SetPosition(glm::vec3(1040.0f, 250.0f + 200.0f * cos(time / 1000.0f), 300.0f + 200.0f * sin(time / 1000.0f)));
			}
			else if (sceneTime < 9000) {
				lights[0]->SetPosition(glm::vec3(1500.0f + 100.0f * sin(3 * time / 1000.0f), 50.0f + 40.0f * cos(2 * time / 1000.0f), 1200.0f + 200.0f * sin(time / 1000.0f)));
			}
			else {
				lights[0]->SetPosition(glm::vec3(1500.0f + 100.0f * sin(time / 1000.0f), 50.0f, 500.0f + 50.0f * sin(time / 1000.0f)));
			}
		
		}

		if (sceneTime > 20000) {
			postProcessor->ShatterMoveOn();
			postProcessor->Update(msec / 1000.0f);
		}

		// Can't get these stupid shadows working. Last try...

		if (!lights.empty()) {
			std::vector<glm::mat4> shadowTransforms;
			glm::vec3 lightPos = glm::vec3(lights[0]->GetPosition());
			//lightPos = glm::vec3(1300.0f, 50.0f, 300.0f);
			//shadowProj = projMatrix;
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));

		//projMatrix = shadowProj;
		//viewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		/*	SHADER_MANAGER->SetShader("ShadowDepth");
			for (int i = 0; i < 6; ++i) {
				SHADER_MANAGER->SetUniform("ShadowDepth", "shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
			}
			SHADER_MANAGER->SetUniform("ShadowDepth", "lightWorldPos", lightPos);
			SHADER_MANAGER->SetUniform("ShadowDepth", "farPlane", farPlane);*/
			depthShader->Use();
			for (int i = 0; i < 6; ++i) {
				glUniformMatrix4fv(shadowMatrixLocations[i], 1, GL_FALSE, &shadowTransforms[i][0][0]);
			}
			GLuint lightPosLocation = glGetUniformLocation(depthShader->Program, "lightPos");
			glUniform3fv(lightPosLocation, 1, &lightPos[0]);
			shadowTransforms.clear();
		}


	}
	if (currentScene == SCENE_C) {
		flareManager->PrepareToRender(camera->GetPosition(), projMatrix * viewMatrix, sun->GetPosition());

		//coneNode->SetTransform(glm::rotate(rotation, rotationAxis) * coneNode->GetTransform());
		//glm::vec3 startingDir = glm::normalize(glm::vec3(spotlight->GetDirection()));
		spotlight->Randomise(msec);
		rotationAngle = 17.5f * sin(sceneTime / 750.0f);
		glm::mat4 rot = glm::rotate(glm::radians(rotationAngle), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::radians(rotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
		coneNode->SetTransform(rot);
		spotlight->Rotate(rot);
		
		//glm::vec3 currentDir = glm::normalize(glm::vec3(spotlight->GetDirection()));
		//glm::vec3 rotationAxis = glm::cross(startingDir, currentDir);
		//if (glm::length2(rotationAxis) > 0.01) {
		//	float rotation = glm::asin(rotationAxis.length());
		//	coneNode->SetTransform(glm::rotate(rotation, rotationAxis) * coneNode->GetTransform());
		//
		//}
	////InProgress: Rotate cone node depending on direction (check without randomise first)
											// Step2: Send colour to shader
											// Step3: have a go at changing size
											

	}
	if (currentScene == SCENE_D) {
		if (sceneTime < 10000) {
			ufoNode->SetTransform(glm::translate(laser->GetPosition() + glm::vec3(0.0, 500.0f, 0.0f)));
			laser->SetActive();
			SHADER_MANAGER->SetUniform("HoleTerrainShader", "holeTime", sceneTime / 10.0f);
		}
		else if (sceneTime < 15000){
			const float UFO_SPEED = 0.0005f; // Units per milliseconds

			glm::vec3 targetPosition = glm::vec3(1000.0f, 150.0f, 1000.0f);
			glm::vec3 movementDirection = targetPosition - ufoNode->GetPosition();

			ufoNode->SetTransform(glm::translate(glm::vec3(UFO_SPEED * msec * movementDirection)) * ufoNode->GetTransform());
			laser->SetInactive();
			
		}
		else if (sceneTime < 15100) {
			glm::vec3 ufoPos = ufoNode->GetPosition();
			laser->SetTransform(glm::translate(ufoPos - glm::vec3(0.0, 500.0f, 0.0f)));
			laser->SetActive();
			SHADER_MANAGER->SetUniform("HoleTerrainShader", "epicentre2", glm::vec2(ufoPos.x, ufoPos.z));
			SHADER_MANAGER->SetUniform("HoleTerrainShader", "holeTime2", (sceneTime - 15100) / 10.0f);
		}
		else{
			SHADER_MANAGER->SetUniform("HoleTerrainShader", "holeTime2", (sceneTime - 15100) / 10.0f);
		}

		if (sceneTime > 25000) {
			camera->SetPosition(glm::vec3(-100.0f, 30.0f, 18.0f));
			controller->ToggleAutoMovement();
			Transition(SCENE_D, SCENE_E);
		}

		if (sceneTime > 12000 && sceneTime < 12050) {
			scenes[currentScene]->EmitterOn();
		}
	
		

		/*if (sceneTime > 6750 && !hellKnightNode->IsIdle()) {
			hellKnightNode->SetIdle(true);
			hellKnightNode->PlayAnim(MESHDIR"idle2.md5anim");
			scenes[currentScene]->ToggleEmitter();
		}
		else if (sceneTime > 12000) {
			hellKnightNode->SetShader("ExplodeAnimShader");
		}*/

		
	}

	if (currentScene == SCENE_D && lightning) {
		ambientStrength = 0.2f * lightning->GetDimRatio();
		tempLights = lightning->GetLights();
	}

	if (currentScene == SCENE_C && ufoNode) {
    	const float UFO_SPEED = 0.001f; // Units per milliseconds

		glm::vec3 targetPosition = ufoTargetPositions[ufoTargetIndex];
		glm::vec3 movementDirection = targetPosition - ufoNode->GetPosition();

		if (glm::distance(targetPosition, ufoNode->GetPosition())< 5.0f) {
			SwitchUFOTargetPosition();
			ufoNode->SetTransform(glm::translate(glm::vec3(-2.0f * UFO_SPEED * msec *  movementDirection)) * ufoNode->GetTransform());
		}
	
		ufoNode->SetTransform(glm::translate(glm::vec3(UFO_SPEED * msec * movementDirection)) * ufoNode->GetTransform());
	} 

	if (currentScene == SCENE_D && grass) {
		grass->Update(msec);
		SHADER_MANAGER->SetUniform("Grass", "modelMatrix", modelMatrix);
		SHADER_MANAGER->SetUniform("Grass", "viewMatrix", viewMatrix);
		SHADER_MANAGER->SetUniform("Grass", "projMatrix", projMatrix);
	}

}

void Renderer::PrepareToTransition() {
	if (sceneTime > (SCENE_TIME - SCENE_TRANSITION_TIME / 2.0)) {
		postProcessor->SetBlurPasses(1 + 3 * round(sceneTime / SCENE_TRANSITION_TIME));
		postProcessor->BlurOn(); 
	}
	if (sceneTime > SCENE_TIME) {
		SceneNumber nextScene = SceneNumber((currentScene + 1) % SceneNumber::NUM_SCENES);
		Transition(currentScene, nextScene);
	}
}

void Renderer::PostTransition() {
	if (sceneTime < SCENE_TRANSITION_TIME / 2.0) {
		postProcessor->SetBlurPasses(4 - 3 * round((2.0 * sceneTime) / SCENE_TRANSITION_TIME)); // Decrease blur over time
	}
	 
	if (sceneTime > SCENE_TRANSITION_TIME / 2.0 && 
		sceneTime < SCENE_TIME - SCENE_TRANSITION_TIME / 2.0) {
		postProcessor->BlurOff(); 
	}
}

void Renderer::SwitchUFOTargetPosition() { //Temporary function delete
	ufoTargetIndex = ++ufoTargetIndex % 2;
}

void Renderer::SimpleShadowFirstPass() {
	simpleShadow->BindShadow();
	//
	RenderObjects(NO_CLIP_PLANE);
	//
	simpleShadow->UnbindShadow();
}

void Renderer::DrawSimpleShadowScene() {
	simpleShadow->BindForReading();
	RenderObjects(NO_CLIP_PLANE);
}

void Renderer::TestDraw() {
	// don't forget to enable shader before setting uniforms
	shaderAI->Use();

	// view/projection transformations
	GLint lightPosLocation = glGetUniformLocation(shaderAI->Program, "pointLights[0].position");
	GLint lightColLocation = glGetUniformLocation(shaderAI->Program, "pointLights[0].colour");
	GLint lightRadLocation = glGetUniformLocation(shaderAI->Program, "pointLights[0].radius");
	if (!lights.empty()) {
		bulbNode->SetTransform(glm::translate(glm::vec3(lights[0]->GetPosition())));
		glUniform4fv(lightPosLocation, 1, (float*)&lights[0]->GetPosition());
		glUniform4fv(lightColLocation, 1, (float*)&lights[0]->GetColour());
		glUniform1f(lightRadLocation, lights[0]->GetRadius());
	}


	GLint viewLocation = glGetUniformLocation(shaderAI->Program, "viewMatrix");
	GLint projectionLocation = glGetUniformLocation(shaderAI->Program, "projectionMatrix");
	if (viewLocation < 0 || projectionLocation < 0)
	{
		std::cout << std::endl << glGetError() << std::endl;
		exit(-88);
	}
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projMatrix));
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));


	// render the loaded model
	GLint modelLocation = glGetUniformLocation(shaderAI->Program, "modelMatrix");
	if (modelLocation < 0)
	{
		std::cout << std::endl << glGetError() << std::endl;
		exit(-77);
	}

	for (int i = 0; i < 2; ++i) {
		glm::mat4 modelMat = glm::translate(glm::vec3((i + 5) * 200.0f, 100.0f * (1.5 - i), 60.0f * (i + 5))); // translate it down so it's at the center of the scene
		modelMat = modelMat * glm::scale(glm::vec3(200.0f, 200.0f, 200.0f));	// it's a bit too big for our scene, so scale it down
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(modelMat));
		text3D[i]->Draw(*shaderAI);
	}

	glm::mat4 modelMat = glm::translate(glm::vec3((8.2) * 200.0f, 100.0f * (0.5), 60.0f * (18))); // translate it down so it's at the center of the scene
	modelMat = modelMat * glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	modelMat = modelMat * glm::scale(glm::vec3(200.0f, 200.0f, 200.0f));	// it's a bit too big for our scene, so scale it down
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(modelMat));
	text3D[2]->Draw(*shaderAI);

	SHADER_MANAGER->SetUniform("TerrainShadowShader", "depthMapTex", 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	RenderObjects(NO_CLIP_PLANE);
}