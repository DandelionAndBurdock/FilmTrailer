#include "Lines.h"



Lines::Lines()
{
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);
}


Lines::~Lines()
{
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

void Lines::AddLine(glm::vec2 p1, glm::vec2 p2) {
	lineVerts.push_back(p1);
	lineVerts.push_back(p2);
}

void Lines::Buffer() {
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, lineVerts.size() * sizeof(glm::vec2), &lineVerts[0], GL_STATIC_DRAW);
}
void Lines::Draw() {
	glLineWidth(8);
	
}
