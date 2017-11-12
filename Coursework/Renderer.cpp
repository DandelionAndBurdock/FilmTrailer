#include "Renderer.h"

#include "glm/gtx/transform.hpp"
#include "nclgl/HeightMap.h"
#include "nclgl/TextureManager.h"
#include "nclgl/ShaderManager.h"
#include "nclgl/TextRenderer.h"
#include "nclgl\PerlinNoise.h"
#include "nclgl\ParticleSystem.h"
#include "nclgl\ParticleManager.h"


#include <iostream>
#include <iomanip> // setprecision()


Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	CubeRobot::CreateCube();

	
	projMatrix = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 1.0f, 10000.0f);


	SetupCamera();
	LoadShaders();
	LoadTextures();

	SetupScenes();
	currentRoot = sceneARoot;

	text = new TextRenderer(width, height, FONTSDIR"arial.ttf", 30, glm::vec3(1.0f, 1.0f, 1.0f));

	//SetupLine();
	//PerlinNoise p;
	//p.GenerateTexture();



	ConfigureOpenGL();

	init = true;
}


Renderer::~Renderer() {
	if (currentRoot)
		delete currentRoot;
	delete sceneARoot;
	delete quad;
	delete camera;
	delete cameraControl;
	delete particleSystem;
	//delete particleManager;
	CubeRobot::DeleteCube();
}


void Renderer::SetupSceneA() {
	quad = Mesh::GenerateQuad();


	terrain = new HeightMap(TEXTUREDIR"terrain.raw");

	sceneARoot = new SceneNode();
	sceneARoot->AddChild(terrain);
	terrain->UseTexture("Terrain");
	terrain->SetShader("TerrainShader");

	for (int i = 0; i < 5; ++i) {
		SceneNode* s = new SceneNode();
		s->SetColour(glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));
		s->SetTransform(glm::translate(glm::vec3(0.0f, 100.0f, -200.0f + 100.0f * i)));
		s->SetModelScale(glm::vec3(100.0f, 100.0f, 100.0f));
		s->SetBoundingRadius(100.0f);
		s->SetMesh(quad);
		s->UseTexture("StainedGlass");
		s->SetShader("QuadShader");

		terrain->AddChild(s);
	}


	terrain->AddChild(new CubeRobot());

	CubeRobot* CR2 = new CubeRobot();
	CR2->SetTransform(glm::translate(glm::vec3(-50.0f)) * glm::scale(glm::vec3(2.0f)));
	terrain->AddChild(CR2);

	particleSystem = new ParticleSystem(glm::vec3(300.0f, 300.0f, 300.0f));
	//particleManager = new ParticleManager();
}

void Renderer::UpdateScene(float msec) {
	CalculateFPS(msec); 

	camera->UpdateCamera(msec);
	particleSystem->UpdateParticles(msec);
	//particleManager->Update(msec, camera->GetPosition());
	//cameraControl->Update(msec);
	viewMatrix = camera->BuildViewMatrix(); //TODO: Move camera construction to cameraControl
	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	if (currentRoot) {
		currentRoot->Update(msec);

		BuildNodeLists(currentRoot);
		SortNodeLists();
	}

	UpdateUniforms();

	
}

void Renderer::BuildNodeLists(SceneNode* from) {
	if (from->GetMesh() && frameFrustum.InsideFrustum(*from)) {
		glm::vec3 translation = glm::vec3(from->GetWorldTransform()[3]);
		glm::vec3 dir = translation - camera->GetPosition();

		from->SetCameraDistance(glm::dot(dir, dir));

		if (from->GetMesh()) {
			activeShaders.insert(from->GetShaderName());
		}

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


void Renderer::RenderScene() {
	//currentShader = ShaderManager::GetInstance()->GetShader("LineShader");
	//currentShader->Use();
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


	DrawNodes();
	DrawFPS();

	//SHADER_MANAGER->SetUniform("Particle", "viewProjMatrix", projMatrix * viewMatrix);
	//SHADER_MANAGER->SetUniform("Particle", "cameraRight", camera->GetRight());
	//SHADER_MANAGER->SetUniform("Particle", "cameraPos", camera->GetPosition());
	//TODO: Add to uniforms
	particleSystem->Render(projMatrix * viewMatrix, camera->GetPosition());
	//DrawLine();
	//currentShader = ShaderManager::GetInstance()->GetShader("QuadShader");
	//currentShader->Use();
	//UpdateShaderMatrices();
	//Texture* tex = TextureManager::GetInstance()->GetTexture("Noise");
	//tex->Bind();
	//quad->Draw();
	SwapBuffers();
	glUseProgram(0);
	ClearNodeLists();
	activeShaders.clear();

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

void Renderer::SetupLine() {
	GLuint VBOSample;
	std::cout << glGetError() << std::endl;
	GLfloat sample[] = {  200.0f, 800.0f, -100.0f ,
	 200.0f, 500.0f, -100.0f ,
	  200.0f, 300.0f, -100.0f ,
	200.0f, 0.0f, -100.0f , 
		200.0f, -300.0f, -100.0f ,
	 200.0f, -600.0f, -100.0f  };
	GLfloat normals[] = { 1.0f, 0.0f, 0.0f ,
		1.0f, 0.0f, 0.0f ,
		1.0f, 0.0f, 0.0f ,
		1.0f, 0.0f, 0.0f ,
		1.0f, 0.0f, 0.0f ,
		1.0f, 0.0f, 0.0f };
	glGenVertexArrays(1, &VAOSample);
	glGenBuffers(1, &VBOSample);
	GLuint VBONormal;

	glBindVertexArray(VAOSample);
	glBindBuffer(GL_ARRAY_BUFFER, VBOSample);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sample), sample, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0); //point start, #
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBONormal);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0); //point start, #
	glEnableVertexAttribArray(1);

}

void Renderer::DrawLine() {
	//Position -- TODO:Remove magic numbers
	glBindVertexArray(VAOSample);
	glDrawArrays(GL_LINE_STRIP, 0, 6);

}

void Renderer::SetupScenes() {
	SetupSceneA();
}


void Renderer::LoadShaders() {
	SHADER_MANAGER->AddShader("TextShader", SHADERDIR"TextVertex.glsl", SHADERDIR"TextFragment.glsl");
	SHADER_MANAGER->AddShader("TerrainShader", SHADERDIR"TexturedColourVertex.glsl", SHADERDIR"TexturedColourFragment.glsl");
	SHADER_MANAGER->AddShader("QuadShader", SHADERDIR"SceneVertex.glsl", SHADERDIR"SceneFragment.glsl");

	//ShaderManager::GetInstance()->AddShader("LineShader", SHADERDIR"BasicVertex.glsl", SHADERDIR"BasicFragment.glsl", SHADERDIR"LineGeometry.glsl");
}

void Renderer::LoadTextures() {
	TEXTURE_MANAGER->AddTexture("StainedGlass", TEXTUREDIR"stainedglass.tga");
	TEXTURE_MANAGER->AddTexture("Terrain", TEXTUREDIR"Barren Reds.jpg");
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


void Renderer::UpdateUniforms() {
	for (const auto& shader : activeShaders) {
		std::vector<std::string> uniforms = SHADER_MANAGER->GetUniformNames(shader);
		for (const auto& uniform : uniforms) {
			if (uniform.find("Tex") != std::string::npos) {
				continue; // Don't handle textures here
			}
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
			else {
				std::cout << "Warning: " << uniform << " was not set by renderer" << std::endl;
			}
		}
	}

}