#pragma once

#include "Plane.h"
#include "../../glm/mat4x4.hpp"
#include "SceneNode.h"

class Matrix4;

class Frustum
{
public:
	Frustum() {};
	~Frustum() {};

	void FromMatrix(const glm::mat4& mvp);
	bool InsideFrustum(SceneNode& n);

protected:
	Plane planes[6];
};
