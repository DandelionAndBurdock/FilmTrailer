#pragma once

#include "OGLRenderer.h" //TODO: Why this include?

enum MeshBuffer {
	VERTEX_BUFFER, COLOUR_BUFFER, TEXTURE_BUFFER, NORMAL_BUFFER, 
	TANGENT_BUFFER, INDEX_BUFFER, MAX_BUFFER
};

class Mesh
{

public:
	Mesh();
	~Mesh();

	virtual void Draw();

	static Mesh* GenerateTriangle();
	static Mesh* GenerateQuad();
	static Mesh* GenerateNullQuad();
	static Mesh* GenerateLine();

	void BufferVertices(glm::vec3* newVertices);

	// Estimate the bounding radius based on the vertices of this mesh
	GLfloat CalculateBoundingRadius();
protected:
	void GenerateTangents();
	void GenerateNormals();

	glm::vec3 GenerateTangent(const glm::vec3& a, const glm::vec3& b,
		const glm::vec3& c, const glm::vec2& ta,
		const glm::vec2& tb, const glm::vec2& tc);

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

	glm::vec3* tangents;
};