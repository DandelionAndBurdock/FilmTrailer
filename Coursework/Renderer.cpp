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

#include <algorithm> // For min()
#include <iostream>
#include <iomanip> // setprecision()


Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	dirLight = nullptr;
	spotlight = nullptr;
	CubeRobot::CreateCube();
	Light::CreateLightMesh();
	

	SetupCamera();
	LoadShaders();
	LoadTextures();

	SetupScenes();
	currentRoot = sceneARoot;

	text = new TextRenderer(width, height, FONTSDIR"arial.ttf", 30, glm::vec3(1.0f, 1.0f, 1.0f));

	//PerlinNoise p;
	//p.GenerateTexture();

	lightning = new Lightning(glm::vec3(500.0, 500.0, 0.0), glm::vec3(200.0, 0.0, 200.0));

	ConfigureOpenGL();

	init = true;

	reflectionQuad = Mesh::GenerateQuad();
	refractionQuad = Mesh::GenerateQuad();
	ambientStrength = 0.2;

	nearPlane = 1.0f;
	farPlane = 10000.0f;
	//omniShadow = new OmniShadow(screenSize.x, screenSize.y);
}


Renderer::~Renderer() {
	//Functions delete meshes...
	if (dirLight)
		delete dirLight;
	if (spotlight)
		delete spotlight;

	delete sceneARoot;
	delete camera;
	delete cameraControl;
	delete particleSystem;
	delete particleManager;
	delete grass;
	delete refractionQuad;
	delete reflectionQuad;
	CubeRobot::DeleteCube();
}


void Renderer::SetupSceneA() {

	terrain = new HeightMap(TEXTUREDIR"terrain.raw");
	SceneNode* heightMap = new SceneNode(terrain, "TerrainShader");
	grass = new Grass(terrain, TEXTUREDIR"grassPack.png");
	sceneARoot = new SceneNode();
	sceneARoot->AddChild(heightMap);
	heightMap->UseTexture("Terrain");
	heightMap->UseTexture("TerrainBump");

	
	//Load meshes function
	OBJMesh* m = new OBJMesh;
	if (m->LoadOBJMesh(MESHDIR"tree_oak.obj")) {
		tree = m;
	}
	else {
		__debugbreak();
	}


	//heightMap->AddChild(new CubeRobot());
	//
	//CubeRobot* CR2 = new CubeRobot();
	//CR2->SetTransform(glm::translate(glm::vec3(-50.0f)) * glm::scale(glm::vec3(2.0f)));
	//heightMap->AddChild(CR2);


	permanentLights.clear();
	//permanentLights.push_back(new Light(glm::vec3(50.0f, 500.0f, 50.0f), glm::vec4(1.0f), 2000.0f));
	//permanentLights.push_back(new Light(glm::vec3(1000.0f, 500.0f, 50.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 2000.0f));
	//permanentLights.push_back(new Light(glm::vec3(1000.0f, 500.0f, 1000.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 2000.0f));
	//permanentLights.push_back(new Light(glm::vec3(500.0f, 200.0f, 2000.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 2000.0f));
	for (auto& light : lights) {
		heightMap->AddChild(light);
	}
	if (dirLight) {
		delete dirLight;
	}
	dirLight = new DirectionalLight(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0,1.0,1.0));
	
	if (spotlight) {
		delete spotlight;
	}
	//spotlight = new Spotlight(glm::vec3(100.0f, 500.0f, 500.0f), glm::vec3(0.0, 0.0, 1.0),glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)));
	//heightMap->AddChild(spotlight);
	water = new Water(GetScreenSize().x, GetScreenSize().y);//TODO: Should water/height map be scene nodes then initialise ?->memory problem //TODO: Not true water height fix
	waterNode = new SceneNode(water, "WaterShader");
	waterNode->SetTransform(glm::translate(glm::vec3(200.0f, water->GetHeight(), 500.0f)) * glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
	waterNode->SetModelScale(glm::vec3(200.0f, 200.0f, 200.0f));
	waterNode->SetColour(glm::vec4(0.0f));// Add to transparent list
	waterNode->UseTexture("Reflection");
	waterNode->UseTexture("WaterBump");
	waterNode->UseTexture("Refraction");
	waterNode->UseTexture("dudvMap");
	waterNode->UseTexture("DepthMap");
	heightMap->AddChild(waterNode);
	SceneNode* t = new SceneNode(tree, "TerrainShader");
	t->SetModelScale(glm::vec3(50.0f));
	t->SetColour(glm::vec4(0.0f));// Add to transparent list
	t->SetTransform(glm::translate(glm::vec3(900.0f, terrain->GetHeightAtPosition(50.0f, 50.0f), 900.0f)));
	heightMap->AddChild(t);
//	heightMap->SetInactive();

	quad = Mesh::GenerateQuad();
	
	//particleSystem = new ParticleSystem(glm::vec3(300.0f, 300.0f, 300.0f));
	particleManager = new ParticleManager();
}

void Renderer::UpdateScene(float msec) {
	projMatrix = glm::perspective(glm::radians(45.0f), (float)width / (float)height, nearPlane, farPlane);
	CalculateFPS(msec); 

	camera->UpdateCamera(msec);
	//particleSystem->UpdateParticles(msec);
	//particleManager->Update(msec, camera->GetPosition());
	//cameraControl->Update(msec);
	viewMatrix = camera->BuildViewMatrix(); //TODO: Move camera construction to cameraControl
	frameFrustum.FromMatrix(projMatrix * viewMatrix);
	//dirLight->Rotate(1.0 / 10.0 * msec, glm::vec3(0.0, 0.0, 1.0));
	grass->Update(msec);
	if (currentRoot) {
		currentRoot->Update(msec);

		BuildNodeLists(currentRoot);
		SortNodeLists();
	}

	//if (lightning) {
	//	lightning->Update(msec);
	//	ambientStrength = 0.2f * lightning->GetDimRatio();
	//	tempLights = lightning->GetLights();
	//}
	lights.clear();
	lights.insert(lights.end(), tempLights.begin(), tempLights.end());
	lights.insert(lights.end(), permanentLights.begin(), permanentLights.end());
	//spotlight->Randomise(msec);
	std::sort(lights.begin(), lights.end(), [](const Light* a, const Light* b) { return *a < *b;});
	UpdateUniforms();
	if (spotlight) {
		spotlight->UpdateTransform();
	}



	for (auto& light : lights) {
		light->UpdateTransform();
	}


	SHADER_MANAGER->SetUniform("Grass", "modelMatrix", modelMatrix);
	SHADER_MANAGER->SetUniform("Grass", "viewMatrix", viewMatrix);
	SHADER_MANAGER->SetUniform("Grass", "projMatrix", projMatrix);
}

void Renderer::RenderObjects(const glm::vec4& clipPlane) {
	for (const auto& shader : activeShaders) {
		SHADER_MANAGER->SetUniform(shader, "clipPlane", clipPlane);
		SHADER_MANAGER->SetUniform(shader, "viewMatrix", camera->BuildViewMatrix()); //TODO: Store
	}
	DrawNodes();
	grass->Draw();
	if (lightning->ShouldFire()) {
		//lightning->Draw(projMatrix * viewMatrix, camera->GetPosition());
	}

}
void Renderer::RenderScene() {
	
	// Enable clipping planes so that we don't have to process geometry
	// above/below the water for refraction/reflection
	glEnable(GL_CLIP_DISTANCE0);

	//*************** WATER**************************
	//TODO: Reflection Pass
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	water->BindReflectionFramebuffer();
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	// To get correct reflection sample we need to mirror (reflect) the camera position/pitch relative to the water plane
	float cameraMirrorShift = camera->GetPosition().y - water->GetHeight(); // Height of camera above the water plane
	camera->Reflect(cameraMirrorShift);
	SHADER_MANAGER->SetUniform("CubeMapShader", "viewMatrix", camera->BuildViewMatrix());
	DrawSkybox();
  	waterNode->SetInactive();
	RenderObjects(glm::vec4(0.0, 1.0, 0.0, -water->GetHeight()));
	//TODO: Refraction Pass
	camera->Reflect(-cameraMirrorShift);
	SHADER_MANAGER->SetUniform("CubeMapShader", "viewMatrix", camera->BuildViewMatrix());
	water->BindRefractionFramebuffer();
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawSkybox();
	RenderObjects(glm::vec4(0.0, -1.0, 0.0, water->GetHeight()));
	water->UnbindFramebuffer(); //Binds Window FBO
	glDisable(GL_CLIP_DISTANCE0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	//ShadowMapFirstPass();


	//omniShadow->BindForReading();
	DrawSkybox();
	waterNode->SetActive();
	RenderObjects(NO_CLIP_PLANE);
	//TODO: Tidy
	//SHADER_MANAGER->SetUniform("Particle", "viewProjMatrix", projMatrix * viewMatrix);
	//SHADER_MANAGER->SetUniform("Particle", "cameraRight", camera->GetRight());
	//SHADER_MANAGER->SetUniform("Particle", "cameraUp", camera->GetUp());

	//particleSystem->Render(projMatrix * viewMatrix, camera->GetPosition());
	particleManager->Render();
	//DrawLine();
	//currentShader = ShaderManager::GetInstance()->GetShader("QuadShader");
	//currentShader->Use();
	//UpdateShaderMatrices();
	//Texture* tex = TextureManager::GetInstance()->GetTexture("Noise");
	//tex->Bind();
	
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
	
	glViewport(300, 0, 200, 200);
	glBindTexture(GL_TEXTURE_2D, water->GetRefractionTex());
	reflectionQuad->Draw();


	// Reset view port
	glViewport(0, 0, screenSize.x, screenSize.y);
	DrawFPS();
	SwapBuffers();
	glUseProgram(0);
	ClearNodeLists();
	activeShaders.clear();

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
		SHADER_MANAGER->SetUniform(n->GetShaderName(), "modelMatrix", n->GetWorldTransform() * glm::scale(n->GetModelScale()));
		n->DrawNode();
	}
}




void Renderer::ClearNodeLists() {
	transparentNodeList.clear();
	opaqueNodeList.clear();

}

void Renderer::DrawFPS() {
	std::stringstream ss;
	//ss << std::fixed << std::setprecision(0) << framesPerSecond;
	ss << framesPerSecond;
	text->RenderText(std::string("FPS: ") + ss.str(), 100, 100, 1.0f);
}

void Renderer::SetupScenes() {
	SetupSceneA();
}


void Renderer::LoadShaders() {
	SHADER_MANAGER->AddShader("TextShader", SHADERDIR"TextVertex.glsl", SHADERDIR"TextFragment.glsl");
	SHADER_MANAGER->AddShader("TerrainShader", SHADERDIR"LightingVertex.glsl", SHADERDIR"LightingFragment.glsl");
	SHADER_MANAGER->AddShader("QuadShader", SHADERDIR"TexturedVertex.glsl", SHADERDIR"TexturedFragment.glsl");
	SHADER_MANAGER->AddShader("LightShader", SHADERDIR"SceneVertex.glsl", SHADERDIR"SceneFragment.glsl");
	SHADER_MANAGER->AddShader("WaterShader", SHADERDIR"WaterVertex.glsl", SHADERDIR"WaterFragment.glsl");
	SHADER_MANAGER->AddShader("CubeMapShader", SHADERDIR"skyboxVertex.glsl", SHADERDIR"skyboxFragment.glsl");
}

void Renderer::LoadTextures() {
	TEXTURE_MANAGER->AddTexture("StainedGlass", TEXTUREDIR"stainedglass.tga");
	TEXTURE_MANAGER->AddTexture("Terrain", TEXTUREDIR"Barren Reds.jpg");
	TEXTURE_MANAGER->AddTexture("TerrainBump", TEXTUREDIR"Barren RedsDOT3.jpg");
	TEXTURE_MANAGER->AddTexture("dudvMap", TEXTUREDIR"waterDUDV.png");
	TEXTURE_MANAGER->AddTexture("waterBump", TEXTUREDIR"waterNormalMap.png");
	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR"lake1_lf.jpg",
		TEXTUREDIR"lake1_rt.jpg",
		TEXTUREDIR"lake1_up.jpg",
		TEXTUREDIR"lake1_dn.jpg",
		TEXTUREDIR"lake1_ft.jpg",
		TEXTUREDIR"lake1_bk.jpg",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0
	);

	if (!cubeMap) {
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
	for (const auto& shader : activeShaders) {
		std::vector<std::string> uniforms = SHADER_MANAGER->GetUniformNames(shader);
		for (const auto& uniform : uniforms) {//TODO: Refactor
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
				else if (uniform == "cubeTex") {
					SHADER_MANAGER->SetUniform(shader, uniform, 0);
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
				else if (uniform == "textureMatrix") {
					SHADER_MANAGER->SetUniform(shader, uniform, textureMatrix);
				}
				else if (uniform == "ambientStrength") {
					SHADER_MANAGER->SetUniform(shader, uniform, ambientStrength);
				}
				else if (uniform == "cameraPos") {
					SHADER_MANAGER->SetUniform(shader, uniform, camera->GetPosition());
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
		if (spotlight) {
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
	omniShadow->BindForWriting();
	omniShadow->SetUniforms(lights[0]);
	projMatrix = glm::perspective(glm::radians(90.0f), screenSize.x / screenSize.y, 1.0f, 100.0f);//TODO: Check far plane
	RenderObjects(NO_CLIP_PLANE);
	omniShadow->Unbind();
}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);
	SHADER_MANAGER->SetShader("CubeMapShader");
	quad->Draw();
	glUseProgram(0);
	glDepthMask(GL_TRUE);
}
