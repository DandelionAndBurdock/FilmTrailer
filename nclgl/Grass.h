#pragma once
#include "../GLEW/include/GL/glew.h"

#include <string>

class HeightMap;
class Shader;
// TOOD: Grass can belong to a height map?
class Grass
{
public:
	Grass(HeightMap* const terrain, std::string imagePath);
	~Grass();

	void Draw();
	void Update(GLfloat msec);
protected:
	void BufferGrassPositions(HeightMap* const terrain);
	GLuint grassVAO;
	// These define the area to randomly place the grass
	// best defined relative to the size of the terrain in 
	// model space
	GLfloat xDimension = 16.0f * 257;
	GLfloat zDimension = 16.0f * 257;

	// Number of grass clustes that have been generated
	int numClusters;

	GLuint textureID;


	float time;

};

