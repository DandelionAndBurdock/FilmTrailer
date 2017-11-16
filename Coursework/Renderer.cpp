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

	lightning = new Lightning(glm::vec3(-500.0, 500.0, 0.0), glm::vec3(0.0, 0.0, 0.0));

	ConfigureOpenGL();

	init = true;

	quad = Mesh::GenerateQuad();
}


Renderer::~Renderer() {
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
	delete quad;
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

	

	//heightMap->AddChild(new CubeRobot());
	//
	//CubeRobot* CR2 = new CubeRobot();
	//CR2->SetTransform(glm::translate(glm::vec3(-50.0f)) * glm::scale(glm::vec3(2.0f)));
	//heightMap->AddChild(CR2);


	lights.clear();
	//lights.push_back(new Light(glm::vec3(50.0f, 500.0f, 50.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 2000.0f));
	//lights.push_back(new Light(glm::vec3(1000.0f, 500.0f, 50.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 2000.0f));
	//lights.push_back(new Light(glm::vec3(1000.0f, 500.0f, 1000.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 2000.0f));
	//lights.push_back(new Light(glm::vec3(500.0f, 200.0f, 2000.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 2000.0f));
	for (auto& light : lights) {
		heightMap->AddChild(light);
	}
	if (dirLight) {
		delete dirLight;
	}
	dirLight = new DirectionalLight(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0));
	
	if (spotlight) {
		delete spotlight;
	}
	spotlight = new Spotlight(glm::vec3(500.0f, 500.0f, 500.0f), glm::vec3(0.0, 0.0,1.0),glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)));
	heightMap->AddChild(spotlight);
	water = new Water(GetScreenSize().x, GetScreenSize().y);//TODO: Should water/height map be scene nodes then initialise ?->memory problem
	SceneNode* waterNode = new SceneNode(water, "LightShader");
	waterNode->SetTransform(glm::translate(glm::vec3(500.0f, 100.0f, 500.0f)) * glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
	waterNode->SetModelScale(glm::vec3(200.0f, 200.0f, 200.0f));
	heightMap->AddChild(waterNode);
	//particleSystem = new ParticleSystem(glm::vec3(300.0f, 300.0f, 300.0f));
//	particleManager = new ParticleManager();
}

void Renderer::UpdateScene(float msec) {
	projMatrix = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 1.0f, 10000.0f);
	CalculateFPS(msec); 

	camera->UpdateCamera(msec);
	//particleSystem->UpdateParticles(msec);
//	particleManager->Update(msec, camera->GetPosition());
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

void Renderer::RenderObjects() {
	DrawNodes();
	grass->Draw();
	lightning->Draw(projMatrix * viewMatrix, camera->GetPosition());
}
void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	
	water->BindReflectionFramebuffer();
	RenderObjects();
	water->UnbindFramebuffer();

	RenderObjects();
	//TODO: Tidy
	//SHADER_MANAGER->SetUniform("Particle", "viewProjMatrix", projMatrix * viewMatrix);
	//SHADER_MANAGER->SetUniform("Particle", "cameraRight", camera->GetRight());
	//SHADER_MANAGER->SetUniform("Particle", "cameraUp", camera->GetUp());

	//particleSystem->Render(projMatrix * viewMatrix, camera->GetPosition());
	//particleManager->Render();
	//DrawLine();
	//currentShader = ShaderManager::GetInstance()->GetShader("QuadShader");
	//currentShader->Use();
	//UpdateShaderMatrices();
	//Texture* tex = TextureManager::GetInstance()->GetTexture("Noise");
	//tex->Bind();
	
	projMatrix = glm::ortho(-1, 1, -1, 1);
	viewMatrix = glm::mat4();
	glViewport(0, 0, 300, 300);
	SHADER_MANAGER->SetShader("QuadShader");
	SHADER_MANAGER->SetUniform("QuadShader", "modelMatrix", glm::mat4());
	SHADER_MANAGER->SetUniform("QuadShader", "viewMatrix", viewMatrix);
	SHADER_MANAGER->SetUniform("QuadShader", "projMatrix", projMatrix);

	SHADER_MANAGER->SetUniform("QuadShader", "diffuseTex", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, water->GetReflectionTex());
	
	quad->Draw();
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
		BuildNodeLists(*iter);
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
		SHADER_MANAGER->SetShader(n->GetShaderName());
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
}

void Renderer::LoadTextures() {
	TEXTURE_MANAGER->AddTexture("StainedGlass", TEXTUREDIR"stainedglass.tga");
	TEXTURE_MANAGER->AddTexture("Terrain", TEXTUREDIR"Barren Reds.jpg");
	TEXTURE_MANAGER->AddTexture("TerrainBump", TEXTUREDIR"Barren RedsDOT3.jpg");
}

void Renderer::ConfigureOpenGL() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::SetupCamera() {
	camera = new Camera();
	camera->SetPosition(glm::vec3(0.0f, 100.0f, 750.0f));
	std::vector<glm::vec3> wps = { glm::vec3(-2000.0f, 1800.0f, -2000.0f), glm::vec3(2000.0f, -1800.0f, -2000.0f) , glm::vec3(2000.0f, 1800.0f, 2000.0f), glm::vec3(-2000.0f, -1800.0f, 2000.0f) }; //TODO: Read in from file
	std::vector<glm::vec3> lps = { glm::vec3(00.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) , glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-0.0f, 0.0f, 0.0f) };
	cameraControl = new CameraController(camera, wps, lps);
}

//TODO: Refactor
void Renderer::UpdateUniforms() {
	for (const auto& shader : activeShaders) {
		std::vector<std::string> uniforms = SHADER_MANAGER->GetUniformNames(shader);
		for (const auto& uniform : uniforms) {//TODO: Refactor
			if (uniform.find("Tex") != std::string::npos) {
				continue; // Don't handle textures here
			}
			if (uniform.find("Light") != std::string::npos) {
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
					SHADER_MANAGER->SetUniform(shader, uniform, 0.2f);//TOOD: Make variable
				}
				else if (uniform == "cameraPos") {
					SHADER_MANAGER->SetUniform(shader, uniform, camera->GetPosition());
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