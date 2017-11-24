#include "Frustum.h"

bool Frustum::InsideFrustum(SceneNode& n) {
	glm::vec3 nodePosition = glm::vec3(n.GetWorldTransform()[3]);
	GLfloat nodeBoundingRadius = n.GetBoundingRadius();
	for (int p = 0; p < 6; ++p) {
		if (!planes[p].SphereInPlane(nodePosition, nodeBoundingRadius))
			return false;
	}

	return true;
}

void Frustum::FromMatrix(const glm::mat4& mat) {
	glm::vec3 xaxis = glm::vec3(mat[0][0], mat[1][0], mat[2][0]);
	glm::vec3 yaxis = glm::vec3(mat[0][1], mat[1][1], mat[2][1]);
	glm::vec3 zaxis = glm::vec3(mat[0][2], mat[1][2], mat[2][2]);
	glm::vec3 waxis = glm::vec3(mat[0][3], mat[1][3], mat[2][3]);

	planes[0] = Plane(waxis - xaxis, (mat[3][3] - mat[3][0]), true); // right
	planes[1] = Plane(waxis + xaxis, (mat[3][3] - mat[3][0]), true); // left
	planes[2] = Plane(waxis + yaxis, (mat[3][3] - mat[3][1]), true); // bottom
	planes[3] = Plane(waxis - yaxis, (mat[3][3] - mat[3][1]), true); // top
	planes[4] = Plane(waxis - zaxis, (mat[3][3] - mat[3][2]), true); // far
	planes[5] = Plane(waxis + zaxis, (mat[3][3] - mat[3][2]), true); // near
}

