#include "SceneNode.h"

#include "TextureManager.h"
#include "Texture.h"

SceneNode::SceneNode(Mesh* m, const std::string& shader)
{	
	this->shader = shader;
	this->mesh = m;
	this->colour = glm::vec4(1.0f);
	parent = nullptr;
	modelScale = glm::vec3(1.0f, 1.0f, 1.0f);
	worldTransform = glm::mat4();
	transform = glm::mat4();
	distanceFromCamera = 0.0f;

	if (mesh) {
		boundingRadius = m->CalculateBoundingRadius();
	}
	else {
		boundingRadius = 0.0f;
	}
	isActive = true;
}


SceneNode::~SceneNode()
{
	for (int i = 0; i < children.size(); ++i) {
		delete children[i];
	}
}

void SceneNode::AddChild(SceneNode* s) {
	children.push_back(s);
	s->parent = this;
}

void SceneNode::DrawNode() {
	if (isActive && mesh) {
		BindTextures();
		mesh->Draw();
	}
}

void SceneNode::Update(float msec) {
	if (!isActive) {
		return;
	}
	
	if (parent) {
		worldTransform = parent->worldTransform * transform;
	}
	else { // Root node
		worldTransform = transform;
	}

	for (auto iter = children.begin(); iter != children.end(); ++iter) {
		(*iter)->Update(msec);
	}
}



void SceneNode::BindTextures() {
	for (int i = 0; i < textures.size(); ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		TEXTURE_MANAGER->BindTexture(textures[i]);
	}
}

float SceneNode::GetBoundingRadius() const {
	float maxScale = glm::max(glm::max(modelScale.x, modelScale.y), modelScale.z);
	return boundingRadius * maxScale;
}

void SceneNode::SetInactive() {
	isActive = false;
	for (auto iter = children.begin(); iter != children.end(); ++iter) {
		(*iter)->SetInactive();
	}
}
void SceneNode::SetActive() {
	isActive = true;
	for (auto iter = children.begin(); iter != children.end(); ++iter) {
		(*iter)->SetInactive();
	}
}