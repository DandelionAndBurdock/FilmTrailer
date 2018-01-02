#pragma once

#include "../glm/vec2.hpp"

#include "../GLEW/include/GL/glew.h"

#include <vector>

class Lines
{
public:
	Lines();
	~Lines();

	void AddLine(glm::vec2 p1, glm::vec2 p2);
	void Buffer();
	void Draw();
protected:
	std::vector<glm::vec2> lineVerts;

	GLuint VBO;
	GLuint VAO;
};

