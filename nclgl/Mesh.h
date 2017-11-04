#pragma once

#include "OGLRenderer.h" //TODO: Why this include?

enum MeshBuffer {
	VERTEX_BUFFER, COLOUR_BUFFER, TEXTURE_BUFFER, NORMAL_BUFFER, INDEX_BUFFER, MAX_BUFFER
};

class Mesh
{

public:
	Mesh();
	~Mesh();

	virtual void Draw();

	static Mesh* GenerateTriangle();
	static Mesh* GenerateQuad();

	
protected:
	void GenerateNormals();

	void BufferData();

	GLuint arrayObject;					// VAO
	GLuint bufferObject[MAX_BUFFER];	// VBO ID's 

	GLuint numVertices;
	GLuint numIndices;
	GLuint type;

	glm::vec3* vertices;
	glm::vec3* normals;
	glm::vec4* colours;
	glm::vec2* textureCoords;
	unsigned int* indices;
};