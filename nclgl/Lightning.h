#pragma once

#include <vector>
#include "../../glm/glm.hpp"

#include "../../GLEW/include/GL/glew.h" //TODO: Derive from MEsh?

class Light;

class Lightning//TODO: Derive mesh
{
	const float BASE_THICKNESS = 2.5;
	const float BASE_NEW_BRANCH = 0.05;
	const int NUM_POINTS = 99;
	const int BOLT_RATE = 5000.0f; // Time in msec between bolts
	const int DARK_TIME = 1000.0f; // Time before bolt ambient lightning should be reduced
	const int DISPLAY_TIME = 300.0f; // Time for bolt to remain on the screen in msec
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

	void Update(GLfloat msec);
	void Generate(const glm::vec3& from, const glm::vec3& to);
	void AddBranch(const glm::vec3& from, const glm::vec3& to, float thickness, int numPoints);
	void Draw(const glm::mat4& viewProj, const glm::vec3 cameraPos);
	GLfloat GetDimRatio() { return dimRatio; }
	bool ShouldFire() { return shouldFire; }
	std::vector<Light*> GetLights() { return lights; }
protected:
	std::vector<glm::vec3> Divide(const glm::vec3& from, const glm::vec3& to, int numSubdivisions);
	std::vector<Point> bolt;

	void AddJitter(std::vector<glm::vec3>& verts);
	std::vector<int>  GetForkPoints(int size, float prob);
	glm::vec3  GetForkDestination(const glm::vec3& parentFrom, const glm::vec3& parentTo, glm::vec3 branchFrom);

	std::vector<int> drawVertices;

	// Vertex Array Object
	GLuint lightningVAO;
	GLuint VBO;

	glm::vec3 motherFrom;
	glm::vec3 motherTo;

	GLfloat lastBoltTime = 0.0f; // Time since last bolt in msec
	GLfloat dimRatio = 0.0f; // Dim light before lightning strike to make it more dramatic
	bool shouldFire = false; // Should display bolt

	std::vector<Light*>  lights; // Point lights representing this bolt
};

