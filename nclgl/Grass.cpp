//http://www.mbsoftworks.sk/index.php?page=tutorials&series=1&tutorial=32

#include "Grass.h"

#include "../glm/vec3.hpp"
#include "RandomNumberGenerator.h"
#include "HeightMap.h"
#include "ShaderManager.h"

Grass::Grass(HeightMap* const terrain, std::string imagePath)
{
	numClusters = 0;
	textureID = SOIL_load_OGL_texture(imagePath.c_str(),
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	if (!textureID) {
		return;
	}
	BufferGrassPositions(terrain);

	SHADER_MANAGER->AddShader("Grass", SHADERDIR"GrassVertex.glsl", SHADERDIR"GrassFragment.glsl", SHADERDIR"GrassGeometry.glsl");


	time = 0.0f;
}


Grass::~Grass()
{
}

void Grass::Update(GLfloat msec) {
	time += msec;
}
void Grass::BufferGrassPositions(HeightMap* const terrain) {
	// Each grass cluster is offset from the previous cluster by 
	// a randomly generated offset between grassCusterOffsetMin 
	// and grassCusterOffsetMax 
	float grassClusterOffsetMin = 3.5f;
	float grassClusterOffsetMax = 5.5f;
	
	// Each grass cluster has a height between grassPatchHeightMin
	// and grassPatchHeightMax
	//TODO: IMPLEMENT
	float grassClusterHeight = 5.0f;

	// Start in the centre 	add clusters while not gone out of the boundaries
	glm::vec3 currentClusterPosition(xDimension * 0.5f, 0.0f, zDimension * 0.5f);
	glm::vec3 startingClusterPosition(xDimension * 0.5f, 0.0f, zDimension * 0.5f);
	std::vector<glm::vec3> vertices;

	//TODO: Make a function InBounds()
	while (currentClusterPosition.x + grassClusterOffsetMax < startingClusterPosition.x + 500.0f && 
		   currentClusterPosition.x > startingClusterPosition.x - 500.0f &&
		 currentClusterPosition.z + grassClusterOffsetMax < startingClusterPosition.z + 500.0f &&
		currentClusterPosition.z > startingClusterPosition.z - 500.0f)
	{
		++numClusters;
		currentClusterPosition.x += RNG->GetRandOffset(grassClusterOffsetMin, grassClusterOffsetMax);
		currentClusterPosition.z += RNG->GetRandOffset(grassClusterOffsetMin, grassClusterOffsetMax);

		currentClusterPosition.y = terrain->GetHeightAtPosition(currentClusterPosition.x, currentClusterPosition.z);

		vertices.push_back(currentClusterPosition);
	}
	GLuint grassVBO;
	glGenVertexArrays(1, &grassVAO);
	glGenBuffers(1, &grassVBO);
	glBindVertexArray(grassVAO);
	glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
	glBufferData(GL_ARRAY_BUFFER, numClusters * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, (void*)0);
}

void Grass::Draw() {
	SHADER_MANAGER->SetShader("Grass");
	SHADER_MANAGER->SetUniform("Grass", "time", time / 1000.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(grassVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glDrawArrays(GL_POINTS, 0, numClusters);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
	glDisable(GL_BLEND);
}