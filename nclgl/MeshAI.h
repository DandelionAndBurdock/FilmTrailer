#pragma once

#include "../glm/glm.hpp"
#include <string>
#include <vector>

#include "ShaderAI.h"


struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
};

struct TextureAI {
	unsigned int id;
	std::string type;
};

class MeshAI
{
public:
	/*  Mesh Data  */
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<TextureAI> textures;
	/*  Functions  */
	MeshAI(const std::vector<Vertex>& vertices,
		const std::vector<unsigned int>& indices,
		const std::vector<TextureAI>& textures);
	void Draw(const ShaderAI& shader) const;
private:
	/*  Render data  */
	unsigned int VAO, VBO, EBO;
	/*  Functions    */
	void SetupMesh();
};