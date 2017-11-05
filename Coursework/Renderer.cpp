#include "Renderer.h"

#include "glm/gtx/transform.hpp"
#include "nclgl/HeightMap.h"
#include "nclgl/TextureManager.h"
#include "nclgl/ShaderManager.h"
#include "nclgl/TextRenderer.h"
#include "nclgl\PerlinNoise.h"

#include "nclgl\Texture.h" //TODO: Remove

#include <iomanip> // setprecision()

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	CubeRobot::CreateCube();

	//projMatrix = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 1.0f, 10000.0f);
	projMatrix = glm::ortho(-float(width) / 2, float(width) / 2, -float(height) / 2, float(height) / 2);

	camera = new Camera();
	camera->SetPosition(glm::vec3(0.0f, 100.0f, 750.0f));
	std::vector<glm::vec3> wps = { glm::vec3(-2000.0f, 1800.0f, -2000.0f), glm::vec3(2000.0f, -1800.0f, -2000.0f) , glm::vec3(2000.0f, 1800.0f, 2000.0f), glm::vec3(-2000.0f, -1800.0f, 2000.0f) }; //TODO: Read in from file
	std::vector<glm::vec3> lps = { glm::vec3(00.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) , glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-0.0f, 0.0f, 0.0f) };
	cameraControl = new CameraController(camera, wps, lps);


	ShaderManager::GetInstance()->AddShader("TerrainShader", SHADERDIR"TexturedColourVertex.glsl", SHADERDIR"TexturedColourFragment.glsl");
	ShaderManager::GetInstance()->AddShader("QuadShader", SHADERDIR"SceneVertex.glsl", SHADERDIR"SceneFragment.glsl");
	
	//ShaderManager::GetInstance()->AddShader("LineShader", SHADERDIR"BasicVertex.glsl", SHADERDIR"BasicFragment.glsl", SHADERDIR"LineGeometry.glsl");
	//SetupLine();
	PerlinNoise p;
	p.GenerateTexture();

	quad = Mesh::GenerateQuad();
	TextureManager::GetInstance()->AddTexture("StainedGlass", TEXTUREDIR"stainedglass.tga");
	TextureManager::GetInstance()->AddTexture("Terrain", TEXTUREDIR"Barren Reds.jpg");
	
	terrain = new HeightMap(TEXTUREDIR"terrain.raw");

	text = new TextRenderer(width, height, FONTSDIR"arial.ttf", 30, glm::vec3(1.0f, 1.0f, 1.0f));

	root = new SceneNode();

	for (int i = 0; i < 5; ++i) {
		SceneNode* s = new SceneNode();
		s->SetColour(glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));
		s->SetTransform(glm::translate(glm::vec3(0.0f, 100.0f, -200.0f + 100.0f * i)));
		s->SetModelScale(glm::vec3(100.0f, 100.0f, 100.0f));
		s->SetBoundingRadius(100.0f);
		s->SetMesh(quad);
		s->UseTexture("StainedGlass");
		s->SetShader("QuadShader");

		root->AddChild(s);
	}


	root->AddChild(new CubeRobot());

	CubeRobot* CR2 = new CubeRobot();
	CR2->SetTransform(glm::translate(glm::vec3(-50.0f)) * glm::scale(glm::vec3(2.0f)));
	root->AddChild(CR2);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	init = true;
}


Renderer::~Renderer() {
	delete root;
	delete quad;
	delete camera;
	delete cameraControl;
	CubeRobot::DeleteCube();
}


void Renderer::UpdateScene(float msec) {
	CalculateFPS(msec); 
	//camera->UpdateCamera(msec);
	//cameraControl->Update(msec);
	//viewMatrix = camera->BuildViewMatrix(); //TODO: Move camera construction to cameraControl
	//frameFrustum.FromMatrix(projMatrix * viewMatrix);

	root->Update(msec);
}

void Renderer::BuildNodeLists(SceneNode* from) {
	//if (frameFrustum.InsideFrustum(*from)) {
	//	glm::vec3 dir = glm::vec3(from->GetWorldTransform()[3]) -
	//		camera->GetPosition();

	//	from->SetCameraDistance(glm::dot(dir, dir));

	//	if (from->GetColour().w < 1.0f) {
	//		transparentNodeList.push_back(from);
	//	}
	//	else {
	//		nodeList.push_back(from);
	//	}
	//}

	//for (auto iter = from->GetChildIteratorStart(); iter != from->GetChildIteratorEnd(); ++iter) {
	//	BuildNodeLists(*iter);
	//}

	if (from->GetColour().w < 1.0f) {
		transparentNodeList.push_back(from);
	}
	else {
		nodeList.push_back(from);
	}
	for (auto iter = from->GetChildIteratorStart(); iter != from->GetChildIteratorEnd(); ++iter) {
		BuildNodeLists(*iter);
	}

}

void Renderer::SortNodeLists() {
	std::sort(transparentNodeList.begin(), transparentNodeList.end(), SceneNode::CompareByCameraDistance);
	std::sort(nodeList.begin(), nodeList.end(), SceneNode::CompareByCameraDistance);
}


void Renderer::DrawNodes() {
	for (auto iter = nodeList.begin(); iter != nodeList.end(); ++iter) {
		DrawNode(*iter);
	}

	for (auto iter = transparentNodeList.rbegin(); iter != transparentNodeList.rend(); ++iter) {
		DrawNode(*iter);
	}
}


void Renderer::DrawNode(SceneNode* n) {
	if (n->GetMesh()) {
		currentShader = ShaderManager::GetInstance()->GetShader(n->GetShaderName());
		currentShader->Use();
		UpdateShaderMatrices();
		currentShader->SetUniform("modelMatrix", n->GetWorldTransform() * glm::scale(n->GetModelScale()));
		n->Draw();
	}
}

void Renderer::RenderScene() {
	//BuildNodeLists(root);
	//SortNodeLists();
	//currentShader = ShaderManager::GetInstance()->GetShader("TerrainShader");
	//currentShader->Use();
	//currentShader = ShaderManager::GetInstance()->GetShader("LineShader");
	//currentShader->Use();
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	//UpdateShaderMatrices();
	//currentShader->SetUniform("diffuseTex", 0);
	//Texture* tex = TextureManager::GetInstance()->GetTexture("Terrain");
	//tex->Bind();
	//terrain->Draw();
	//DrawNodes();
	//DrawFPS();
	//DrawLine();
	currentShader = ShaderManager::GetInstance()->GetShader("QuadShader");
	currentShader->Use();
	UpdateShaderMatrices();
	Texture* tex = TextureManager::GetInstance()->GetTexture("Noise");
	tex->Bind();
	quad->Draw();
	SwapBuffers();
	glUseProgram(0);
	//ClearNodeLists();

}

void Renderer::ClearNodeLists() {
	transparentNodeList.clear();
	nodeList.clear();
}

void Renderer::DrawFPS() {
	std::stringstream ss;
	ss << std::fixed << std::setprecision(0) << FPS;
	text->RenderText(std::string("FPS: ") + ss.str(), 100, 100, 1.0f);
}

#include <iostream>
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