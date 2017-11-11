#include "CubeRobot.h"

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp" // For translate()
Mesh* CubeRobot::cube = nullptr;

CubeRobot::CubeRobot()
{
	//SetMesh(cube);

	//body = new SceneNode(cube, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	body = new SceneNode(cube, "TerrainShader");
	body->SetModelScale(glm::vec3(10.0f, 15.0f, 5.0f));
	body->SetTransform(glm::translate(glm::vec3(0.0f, 35.0f, 0.0f)));
	body->SetBoundingRadius(15.0f);
	AddChild(body);

	//head = new SceneNode(cube, glm::vec4(0, 1, 0, 1));
	head = new SceneNode(cube, "TerrainShader");
	head->SetModelScale(glm::vec3(5, 5, 5));
	head->SetTransform(glm::translate(glm::vec3(0, 30, 0)));
	head->SetBoundingRadius(5.0f);
	body->AddChild(head);

	//leftArm = new SceneNode(cube, glm::vec4(0, 0, 1, 1));
	leftArm = new SceneNode(cube, "TerrainShader");
	leftArm->SetModelScale(glm::vec3(3, -18, 3));
	leftArm->SetTransform(glm::translate(glm::vec3(-12, 30, -1)));
	leftArm->SetBoundingRadius(18.0f);
	body->AddChild(leftArm);


	//rightArm = new SceneNode(cube, glm::vec4(0, 0, 1, 1));
	rightArm = new SceneNode(cube, "TerrainShader");
	rightArm->SetModelScale(glm::vec3(3, -18, 3));
	rightArm->SetTransform(glm::translate(glm::vec3(12, 30, -1)));
	rightArm->SetBoundingRadius(18.0f);
	body->AddChild(rightArm);

	//leftLeg = new SceneNode(cube, glm::vec4(0, 0, 1, 1));
	leftLeg = new SceneNode(cube, "TerrainShader");
	leftLeg->SetModelScale(glm::vec3(3, -17.5, 3));
	leftLeg->SetTransform(glm::translate(glm::vec3(-8, 0, 0)));
	leftLeg->SetBoundingRadius(18.0f);
	body->AddChild(leftLeg);


	//rightLeg = new SceneNode(cube, glm::vec4(0, 0, 1, 1));
	rightLeg = new SceneNode(cube, "TerrainShader");
	rightLeg->SetModelScale(glm::vec3(3, -17.5, 3));
	rightLeg->SetTransform(glm::translate(glm::vec3(8, 0, 0)));
	rightLeg->SetBoundingRadius(18.0f);
	body->AddChild(rightLeg);
}


CubeRobot::~CubeRobot()
{
}

void CubeRobot::Update(float msec) {
	transform = transform * glm::rotate(msec / 100.0f, glm::vec3(0.0f, 1.0f, 0.0f));

	head->SetTransform(head->GetTransform() * glm::rotate(-msec / 100.0f, glm::vec3(0.0f, 1.0f, 0.0f)));
	leftArm->SetTransform(leftArm->GetTransform() * glm::rotate(-msec / 100.0f, glm::vec3(1.0f, 0.0f, 0.0f)));
	rightArm->SetTransform(rightArm->GetTransform() * glm::rotate(msec / 100.0f, glm::vec3(1.0f, 0.0f, 0.0f)));

	SceneNode::Update(msec);
}
