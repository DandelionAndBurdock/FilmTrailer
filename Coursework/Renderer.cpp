#include "Renderer.h"

#include "glm/gtx/transform.hpp"
#include "nclgl/HeightMap.h"
#include "nclgl/TextureManager.h"
#include "nclgl/ShaderManager.h"

#include "nclgl\Texture.h" //TODO: Remove

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	CubeRobot::CreateCube();

	projMatrix = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 1.0f, 10000.0f);

	camera = new Camera();
	camera->SetPosition(glm::vec3(0.0f, 100.0f, 750.0f));

	std::vector<std::string> shaderOne = { SHADERDIR"TexturedColourVertex.glsl", SHADERDIR"TexturedColourFragment.glsl" };
	ShaderManager::GetInstance()->AddShader("TerrainShader", shaderOne);
	std::vector<std::string> shaderTwo = { SHADERDIR"SceneVertex.glsl", SHADERDIR"SceneFragment.glsl" };
	ShaderManager::GetInstance()->AddShader("QuadShader", shaderTwo);

	quad = Mesh::GenerateQuad();
	TextureManager::GetInstance()->AddTexture("StainedGlass", TEXTUREDIR"stainedglass.tga");
	TextureManager::GetInstance()->AddTexture("Terrain", TEXTUREDIR"Barren Reds.jpg");
	
	terrain = new HeightMap(TEXTUREDIR"terrain.raw");


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
	CubeRobot::DeleteCube();
}


void Renderer::UpdateScene(float msec) {
	camera->UpdateCamera(msec);
	viewMatrix = camera->BuildViewMatrix();
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
	BuildNodeLists(root);
	SortNodeLists();
	currentShader = ShaderManager::GetInstance()->GetShader("TerrainShader");
	currentShader->Use();

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	UpdateShaderMatrices();
	currentShader->SetUniform("diffuseTex", 0);
	Texture* tex = TextureManager::GetInstance()->GetTexture("Terrain");
	tex->Bind();
	terrain->Draw();
	DrawNodes();
	SwapBuffers();
	glUseProgram(0);
	ClearNodeLists();

}

void Renderer::ClearNodeLists() {
	transparentNodeList.clear();
	nodeList.clear();
}