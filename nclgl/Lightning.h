#pragma once

#include <vector>
#include "../../glm/glm.hpp"

#include "../../GLEW/include/GL/glew.h" //TODO: Derive from MEsh?
class Lightning//TODO: Derive mesh
{
	const float BASE_THICKNESS = 2.5;
	const float BASE_NEW_BRANCH = 0.05;
	const int NUM_POINTS = 199;
#pragma pack(push, 4)
	struct Point {
		glm::vec3 point;
		GLfloat thickness;
		Point(const glm::vec3& pt, GLfloat width) : point(pt), thickness(width) {}
	};
#pragma pack(pop)
public:
	Lightning(const glm::vec3& from, const glm::vec3& to);
	~Lightning();

	void Generate(const glm::vec3& from, const glm::vec3& to);
	void AddBranch(const glm::vec3& from, const glm::vec3& to, float thickness, int numPoints);
	void Draw(const glm::mat4& viewProj, const glm::vec3 cameraPos);
protected:
	std::vector<glm::vec3> Divide(const glm::vec3& from, const glm::vec3& to, int numSubdivisions);
	std::vector<Point> bolt;

	void AddJitter(std::vector<glm::vec3>& verts);
	std::vector<int>  GetForkPoints(int size, float prob);
	glm::vec3  GetForkDestination(const glm::vec3& parentFrom, const glm::vec3& parentTo, glm::vec3 branchFrom);

	std::vector<int> drawVertices;

	// Vertex Array Object
	GLuint lightningVAO;

	glm::vec3 motherFrom;
	glm::vec3 motherTo;

};

