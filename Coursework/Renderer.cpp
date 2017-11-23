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


#include <algorithm> // For min()
#include <iostream>
#include <iomanip> // setprecision()


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

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return;
}

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
	//Light::DeleteLightMesh();
}


void Renderer::SetupSceneA() {
	SceneNode* heightMap = new SceneNode(new HeightMap, "TerrainShadowShader");
	
	scenes.push_back(new Scene(masterRoot));
	scenes[SCENE_A]->SetCubeMap(cubeMapA);
	scenes[SCENE_A]->SetTerrain(heightMap);
	heightMap->UseTexture("Terrain");
	heightMap->UseTexture("TerrainBump");

	
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
	
	//heightMap->AddChild(spotlight);
	
	SceneNode* t = new SceneNode(tree, "TerrainShader");
	t->SetModelScale(glm::vec3(50.0f));
	t->SetColour(glm::vec4(0.0f));// Add to transparent list
	t->SetTransform(glm::translate(glm::vec3(900.0f, 100.0f, 900.0f)));
	heightMap->AddChild(t);


	CubeRobot* cubey = new CubeRobot();
	cubey->SetTransform(glm::translate(glm::vec3(600.0f, -100.0f, 400.0f)) * glm::scale(glm::vec3(4.0f)));
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
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_P)) {
		pause = !pause;
	}
}

void Renderer::SetupSceneB() {
	SceneNode* heightMap = new SceneNode(new HeightMap(TEXTUREDIR"terrain.raw"), "TerrainShader");

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

	
	//scenes[SCENE_B]->AddLight(new Light(glm::vec3(500.0f, 200.0f, 500.0f), glm::vec4(1.0f), 2000.0f));
	//scenes[SCENE_B]->AddLight(new Light(glm::vec3(1000.0f, 500.0f, 50.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 2000.0f));
	//scenes[SCENE_B]->AddLight(new Light(glm::vec3(1000.0f, 500.0f, 1000.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 2000.0f));
	//scenes[SCENE_B]->AddLight(new Light(glm::vec3(500.0f, 200.0f, 2000.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 2000.0f));

	ufoNode = new CubeRobot();
	ufoNode->SetTransform(glm::translate(glm::vec3(300.0f)));
	spotlight = new Spotlight(glm::vec3(0.0), glm::vec3(0.0, 0.0, 1.0), glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)));
	heightMap->AddChild(ufoNode);
	ufoNode->AddChild(spotlight);
	heightMap->AddChild(new CubeRobot());


}

void Renderer::SetupSceneC() {
	scenes.push_back(new Scene(masterRoot));
	scenes[SCENE_C]->SetCubeMap(cubeMapC);
	SceneNode* heightMap = new SceneNode(new HeightMap(TEXTUREDIR"PoolMap.data"), "TerrainShader");
	scenes[SCENE_C]->SetTerrain(heightMap);
	heightMap->UseTexture("Terrain");
	heightMap->UseTexture("TerrainBump");

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


	oceanMesh = new GerstnerWaves;
	oceanNode = new SceneNode(oceanMesh, "GerstnerShader");
	oceanNode->UseTexture("Ocean");
	oceanNode->UseTexture("OceanNormal");
	oceanNode->SetTransform(glm::translate(glm::vec3(0.0f, 200.0f, -3500.0f)));
	heightMap->AddChild(oceanNode);

	CubeRobot* cubey = new CubeRobot();
	cubey->SetTransform(glm::translate(glm::vec3(1500.0f, 300.0f, 2800.0f)) * glm::scale(glm::vec3(2.0f)));
	cubey->UseTexture("Flower");
	heightMap->AddChild(cubey);

	//grass = new Grass(terrain, TEXTUREDIR"grassPack.png");
	cubey = nullptr;
	heightMap = nullptr;

}

void Renderer::SetupSceneD() {
	scenes.push_back(new Scene(masterRoot));
	scenes[SCENE_D]->SetCubeMap(cubeMapD);
	SceneNode* heightMap = new SceneNode(new HeightMap(), "CausticShader");
	scenes[SCENE_D]->SetTerrain(heightMap);
	heightMap->UseTexture("Brick");

	heightMap = nullptr;
}

void Renderer::SetupSceneE() {
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

/*
void Renderer::SetupSceneF(){
scenes.push_back(new Scene(masterRoot));
scenes[SCENE_F]->SetFireworks(new FireworkSystem(glm::vec3(300.0f, 300.0f, 300.0f)));
scenes[SCENE_F]->SetCubeMap(cubeMapC);
*/

void Renderer::UpdateScene(float msec) {
	OGLRenderer::UpdateScene(msec);

	CalculateFPS(msec);

	HandleInput();

	projMatrix = glm::perspective(glm::radians(45.0f), (float)width / (float)height, nearPlane, farPlane);

	camera->UpdateCamera(msec);
	viewMatrix = camera->BuildViewMatrix();

	frameFrustum.FromMatrix(projMatrix * viewMatrix);


	if (!pause) {
		sceneTime += msec;
		if (masterRoot) {
			masterRoot->Update(msec);
		}

		if (sceneTime > 50000.0f) {
			SceneNumber nextScene = SceneNumber((currentScene + 1) % SceneNumber::NUM_SCENES);
			Transition(currentScene, nextScene);
		}

		scenes[currentScene]->UpdateEffects(msec, camera->GetPosition());

		textureMatrix = glm::rotate(glm::radians(time), glm::vec3(0.0, 1.0, 1.0));

		SceneSpecificUpdates(msec);

		hellKnightNode->Update(msec);

	}


	BuildNodeLists(masterRoot);
	SortNodeLists();

	lights.clear();
	lights.insert(lights.end(), tempLights.begin(), tempLights.end());
	lights.insert(lights.end(), scenes[currentScene]->GetLights().begin(), scenes[currentScene]->GetLights().end());

	std::sort(lights.begin(), lights.end(), [](const Light* a, const Light* b) { return *a < *b;});

	UpdateUniforms();

	//particleSystem->UpdateParticles(msec);
	//particleManager->Update(msec, camera->GetPosition());
	//cameraControl->Update(msec);
 //TODO: Move camera construction to cameraControl
	
	//dirLight->Rotate(1.0 / 10.0 * msec, glm::vec3(0.0, 0.0, 1.0));
	//grass->Update(msec);



	
	//spotlight->Randomise(msec);
	
	

	flareManager->PrepareToRender(camera->GetPosition(), projMatrix * viewMatrix, sun->GetPosition());

	//SHADER_MANAGER->SetUniform("Grass", "modelMatrix", modelMatrix);
	//SHADER_MANAGER->SetUniform("Grass", "viewMatrix", viewMatrix);
	//SHADER_MANAGER->SetUniform("Grass", "projMatrix", projMatrix);
}

void Renderer::RenderObjects(const glm::vec4& clipPlane) {
	for (const auto& shader : activeShaders) {
		SHADER_MANAGER->SetUniform(shader, "clipPlane", clipPlane);
		SHADER_MANAGER->SetUniform(shader, "viewMatrix", camera->BuildViewMatrix()); //TODO: Store
	}
	DrawNodes();
	//grass->Draw();

	scenes[currentScene]->DrawEffects(projMatrix * viewMatrix, camera->GetPosition());


}
void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//*************** WATER**************************
	// Enable clipping planes so that we don't have to process geometry
	// above/below the water for refraction/reflection
	if (waterNode->IsActive()) {
		glEnable(GL_CLIP_DISTANCE0);
		SetupReflectionBuffer();
		SetupRefractionBuffer();
		glDisable(GL_CLIP_DISTANCE0);
	}
	//*************** SHADOW *****************************
	ShadowMapFirstPass();
	omniShadow->BindForReading();
	//*************RENDER SCENE **************************
	DrawSceneToBuffer();
	//*************POST PROCESSING **************************
	 postProcessor->ProcessScene();
	PresentScene();

	//*************RENDER GUI**************************
	//RenderReflectionQuad();
	//RenderRefractionQuad();
	//RenderNoiseQuad();
	flareManager->Render();
	DrawFPS();
	scenes[currentScene]->DrawEffects(projMatrix * viewMatrix, camera->GetPosition());
	SwapBuffers();
	glUseProgram(0);
	ClearNodeLists();
	activeShaders.clear();

}

void Renderer::DrawSceneToBuffer() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	postProcessor->BindSceneFBO();
	DrawSkybox();
	RenderObjects(NO_CLIP_PLANE);
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
	glDisable(GL_DEPTH_TEST);
	std::stringstream ss;
	//ss << std::fixed << std::setprecision(0) << framesPerSecond;
	ss << framesPerSecond;
	text->RenderText(std::string("FPS: ") + ss.str(), 100, 100, 1.0f);
	glEnable(GL_DEPTH_TEST);
}

void Renderer::SetupScenes() {
	masterRoot = new SceneNode();
	SetupSceneA();
	SetupSceneB();
	SetupSceneC();
	SetupSceneD();
	SetupSceneE();
	currentCubeMap = cubeMapA;
	currentScene = SCENE_A;
	scenes[SCENE_B]->GetRoot()->SetInactive();
	scenes[SCENE_C]->GetRoot()->SetInactive();
	scenes[SCENE_D]->GetRoot()->SetInactive();
	scenes[SCENE_E]->GetRoot()->SetInactive();
}

void Renderer::LoadShaders() {
	SHADER_MANAGER->AddShader("TextShader", SHADERDIR"TextVertex.glsl", SHADERDIR"TextFragment.glsl");
	SHADER_MANAGER->AddShader("TerrainShadowShader", SHADERDIR"LightingVertex.glsl", SHADERDIR"OmniShadowFrag.glsl");
	SHADER_MANAGER->AddShader("TerrainShader", SHADERDIR"LightingVertex.glsl", SHADERDIR"LightingFragment.glsl");
	//SHADER_MANAGER->AddShader("TerrainShader", SHADERDIR"LightingHeightVertex.glsl", SHADERDIR"LightingFragment.glsl");
	//SHADER_MANAGER->AddShader("TerrainShader", SHADERDIR"LightingVertexMultiTex.glsl", SHADERDIR"LightingFragmentMultiTex.glsl");
	SHADER_MANAGER->AddShader("HoleTerrainShader", SHADERDIR"DeformVertex.glsl", SHADERDIR"DeformFragment.glsl");
	SHADER_MANAGER->AddShader("ShadowDepth", SHADERDIR"ShadowCubeMapVertex.glsl", SHADERDIR"ShadowCubeMapFrag.glsl", SHADERDIR"ShadowCubeMapGeom.glsl");
	SHADER_MANAGER->AddShader("AnimShader", SHADERDIR"AnimVertexNCLGL.glsl", SHADERDIR"AnimFragmentNCLGL.glsl");
	
	SHADER_MANAGER->AddShader("QuadShader", SHADERDIR"TexturedVertex.glsl", SHADERDIR"TexturedFragment.glsl");
	SHADER_MANAGER->AddShader("TextureQuadShader", SHADERDIR"TexturedQuadVertex.glsl", SHADERDIR"TexturedQuadFragment.glsl"); // Adding this one just so I don't break an
	SHADER_MANAGER->AddShader("LightShader", SHADERDIR"SceneVertex.glsl", SHADERDIR"SceneFragment.glsl");
	SHADER_MANAGER->AddShader("WaterShader", SHADERDIR"WaterVertex.glsl", SHADERDIR"WaterFragment.glsl");
	SHADER_MANAGER->AddShader("CubeMapShader", SHADERDIR"skyboxVertex.glsl", SHADERDIR"skyboxFragment.glsl");
	SHADER_MANAGER->AddShader("SunShader", SHADERDIR"SimpleBillBoardVertex.glsl", SHADERDIR"SimpleBillBoardFrag.glsl");
	SHADER_MANAGER->AddShader("FlareShader", SHADERDIR"FlareVertex.glsl", SHADERDIR"FlareFragment.glsl");
	SHADER_MANAGER->AddShader("GerstnerShader", SHADERDIR"GerstnerVertex.glsl", SHADERDIR"GerstnerFragment.glsl");
	SHADER_MANAGER->AddShader("CausticShader", SHADERDIR"CausticVertex3JustCheat.glsl", SHADERDIR"CausticFragment3JustCheat.glsl");
	SHADER_MANAGER->AddShader("ParticleShader", SHADERDIR"vertex.glsl", SHADERDIR"fragment.glsl", SHADERDIR"geometry.glsl");

	// Post processing shaders
	SHADER_MANAGER->AddShader("BlurShader", SHADERDIR"TexturedVertex.glsl", SHADERDIR"BlurFragment.glsl");
	SHADER_MANAGER->AddShader("BloomShader", SHADERDIR"TexturedVertex.glsl", SHADERDIR"BloomFragment.glsl");
	SHADER_MANAGER->AddShader("ContrastShader", SHADERDIR"TexturedVertex.glsl", SHADERDIR"ContrastFragment.glsl");
	SHADER_MANAGER->AddShader("CombineShader", SHADERDIR"TexturedVertex.glsl", SHADERDIR"CombineFragment.glsl");
}

void Renderer::LoadTextures() {
	TEXTURE_MANAGER->AddTexture("StainedGlass", TEXTUREDIR"stainedglass.tga");
	TEXTURE_MANAGER->AddTexture("Terrain", TEXTUREDIR"Barren Reds.jpg");
	TEXTURE_MANAGER->AddTexture("TerrainBump", TEXTUREDIR"Barren RedsDOT3.jpg");
	TEXTURE_MANAGER->AddTexture("dudvMap", TEXTUREDIR"waterDUDV.png");
	TEXTURE_MANAGER->AddTexture("waterBump", TEXTUREDIR"waterNormalMap.png");
	//TEXTURE_MANAGER->AddTexture("Sand", TEXTUREDIR"sand.jpg");
	//TEXTURE_MANAGER->AddTexture("Grass", TEXTUREDIR"grass.jpg");
	TEXTURE_MANAGER->AddTexture("Sand", TEXTUREDIR"rock.jpg");
	TEXTURE_MANAGER->AddTexture("Grass", TEXTUREDIR"rock.jpg");
	TEXTURE_MANAGER->AddTexture("SandGrass", TEXTUREDIR"sandGrass.jpg");
	TEXTURE_MANAGER->AddTexture("Rock", TEXTUREDIR"rock.png");
	TEXTURE_MANAGER->AddTexture("Flower", TEXTUREDIR"Flower.png");
	TEXTURE_MANAGER->AddTexture("Ocean", TEXTUREDIR"Ocean.png");
	TEXTURE_MANAGER->AddTexture("OceanNormal", TEXTUREDIR"ocean_normal.png");
	TEXTURE_MANAGER->AddTexture("Brick", TEXTUREDIR"brick.tga");
	TEXTURE_MANAGER->AddTexture("Explosion", TEXTUREDIR"ParticleAtlas.png");
	TEXTURE_MANAGER->AddTexture("Laser", TEXTUREDIR"blueLaser.jpg");

	TEXTURE_MANAGER->AddTexture("Sun", TEXTUREDIR"sun.png");
	for (int i = 1; i <= 8; ++i) {
		TEXTURE_MANAGER->AddTexture(std::string("Flare") + std::to_string(i), TEXTUREDIR + std::string("Flare") + std::to_string(i) + std::string(".png"));
	}
	
	cubeMapA = SOIL_load_OGL_cubemap(
		TEXTUREDIR"lake1_lf.jpg",
		TEXTUREDIR"lake1_rt.jpg",
		TEXTUREDIR"lake1_up.jpg",
		TEXTUREDIR"lake1_dn.jpg",
		TEXTUREDIR"lake1_ft.jpg",
		TEXTUREDIR"lake1_bk.jpg",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0
	);

	if (!cubeMapA) {
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

	cubeMapC = SOIL_load_OGL_cubemap(
		TEXTUREDIR"rusted_west.jpg",
		TEXTUREDIR"rusted_east.jpg",
		TEXTUREDIR"rusted_up.jpg",
		TEXTUREDIR"rusted_down.jpg",
		TEXTUREDIR"rusted_south.jpg",
		TEXTUREDIR"rusted_north.jpg",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0
	);

	if (!cubeMapC) {
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
		__debugbreak();
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
	cameraControl = new CameraController(camera, wps, lps);
}


// This was an attempt to make setting uniforms nicer but spiralled into a horrible mess
void Renderer::UpdateUniforms() {
	activeShaders.insert("CubeMapShader");
	if (currentScene == SCENE_E) {
		activeShaders.insert("ParticleShader");
	}
	if (currentScene == SCENE_A) {
		activeShaders.insert("Particle");
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
					SHADER_MANAGER->SetUniform(shader, uniform, 3);
				}
				else if (uniform == "sandGrassTex") {
					SHADER_MANAGER->SetUniform(shader, uniform, 4);
				}
				else if (uniform == "grassTex") {
					SHADER_MANAGER->SetUniform(shader, uniform, 5);
				}
				else if (uniform == "rockTex") {
					SHADER_MANAGER->SetUniform(shader, uniform, 6);
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
	farPlane = 10000.0f;
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
	SHADER_MANAGER->SetUniform("QuadShader", "viewMatrix", glm::lookAt(pos, pos + viewDirection, glm::vec3(0, 1, 0)));
	RenderObjects(NO_CLIP_PLANE);
}

void Renderer::RenderSplitScreen(GLuint windowX, GLuint windowY, GLuint windowWidth, GLuint windowHeight) {
	glViewport(windowX, windowY, windowWidth, windowHeight);
	SHADER_MANAGER->SetUniform("QuadShader", "modelMatrix", glm::mat4());
	SHADER_MANAGER->SetUniform("QuadShader", "viewMatrix", glm::mat4());
	SHADER_MANAGER->SetUniform("QuadShader", "projMatrix", glm::ortho(-1, 1, -1, 1));

	SHADER_MANAGER->SetUniform("QuadShader", "diffuseTex", 0);
	glBindTexture(GL_TEXTURE_2D, multipleViewTex);


	glViewport(0, 0, screenSize.x, screenSize.y);
}

// Temporary ugly function
void Renderer::SceneSpecificUpdates(GLfloat msec) {
	if (currentScene == SCENE_E) {
		if (sceneTime < 10000) {
			laser->SetActive();
			SHADER_MANAGER->SetUniform("HoleTerrainShader", "holeTime", sceneTime / 10.0f);
		}
		else {
			laser->SetInactive();
		}

		if (sceneTime > 6000 && !hellKnightNode->IsIdle()) {
			hellKnightNode->SetIdle(true);
			hellKnightNode->PlayAnim(MESHDIR"idle2.md5anim");
		}
		
	}

	if (currentScene == SCENE_A && lightning) {
		ambientStrength = 0.2f * lightning->GetDimRatio();
		tempLights = lightning->GetLights();
	}

	if (currentScene == SCENE_B && ufoNode) {
		const float UFO_SPEED = 0.1f; // Units per milliseconds
		glm::vec3 targetPosition = glm::vec3(1000.0f, 100.0f, 1000.0f);
		glm::vec3 movementDirection = targetPosition - ufoNode->GetPosition();
		ufoNode->SetTransform(glm::translate(glm::vec3(UFO_SPEED * msec * movementDirection)) * ufoNode->GetTransform());
	}

}
