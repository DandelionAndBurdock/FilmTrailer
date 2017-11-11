#pragma once

#include "../../nclgl/SceneNode.h"
#include "../../nclgl/OBJMesh.h"

class CubeRobot : public SceneNode {
public:
	CubeRobot();
	~CubeRobot();

	virtual void Update(float msec);

	static bool CreateCube() {
		OBJMesh* m = new OBJMesh;
		if (m->LoadOBJMesh(MESHDIR"cube.obj")) {
			cube = m;
			return true;
		}
		return false;
		
	}
	static void DeleteCube() { if (cube) delete cube; }

protected:
	static Mesh* cube;
	SceneNode* head;
	SceneNode* leftArm;
	SceneNode* rightArm;
	SceneNode* leftLeg;
	SceneNode* rightLeg;
	SceneNode* body;
};