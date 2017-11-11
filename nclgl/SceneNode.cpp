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
	boundingRadius = 1.0f;
	distanceFromCamera = 0.0f;
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
	if (mesh) {
		BindTextures();
		mesh->Draw();
	}
}

void SceneNode::Update(float msec) {
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

void CalculateBoundingRadius(Mesh* m) {

}

void SceneNode::BindTextures() {
	for (int i = 0; i < textures.size(); ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		TEXTURE_MANAGER->BindTexture(textures[i]);
	}
}