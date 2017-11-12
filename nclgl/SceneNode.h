#pragma once


#include "Mesh.h"

#include <vector>

class SceneNode {
public:
	SceneNode(Mesh* m = nullptr, const std::string& shader = "None");
	~SceneNode();

	void SetTransform(const glm::mat4& matrix) {	transform = matrix; }
	const glm::mat4& GetTransform() const {	return transform; }
	glm::mat4 GetWorldTransform() const {	return worldTransform; }

	glm::vec4 GetColour() const {	return colour; }
	void SetColour(glm::vec4 c) {	colour = c; }

	glm::vec3 GetModelScale() const {	return modelScale; }
	void SetModelScale(glm::vec3 s) {	modelScale = s; }

	Mesh* GetMesh() const {	return mesh; }
	void SetMesh(Mesh* m) {mesh = m;}

	void AddChild(SceneNode* s);

	virtual void Update(float msec);
	virtual void DrawNode();

	std::vector<SceneNode*>::const_iterator GetChildIteratorStart() { return children.cbegin(); }
	std::vector<SceneNode*>::const_iterator GetChildIteratorEnd() { return children.cend(); }

	float GetBoundingRadius() const;
	void SetBoundingRadius(float f) { boundingRadius = f; }

	float GetCameraDistance() const { return distanceFromCamera; }
	void SetCameraDistance(float f) { distanceFromCamera = f; }

	void UseTexture(std::string texture) { textures.push_back(texture); }
	void SetShader(std::string name) { shader = name; }
	const std::string& GetShaderName() { return shader; }




	static bool CompareByCameraDistance(SceneNode* a, SceneNode* b) {
		return a->distanceFromCamera < b->distanceFromCamera ? true : false;
	}

protected:
	void BindTextures();

	SceneNode* parent;
	Mesh* mesh;
	glm::mat4 worldTransform;
	glm::mat4 transform;
	glm::vec3 modelScale;
	glm::vec4 colour;
	std::vector<SceneNode*> children;

	std::vector<std::string> textures;		// List of texures applied to this node
	std::string shader;						// Current shader associated with this node

	float distanceFromCamera;
	float boundingRadius;
};