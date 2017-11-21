#include "HeightMap.h"

#include <iostream>
#include <fstream>

HeightMap::HeightMap(std::string name) 
{

	std::ifstream file(name.c_str(), std::ios::binary);

	if (!file) {
		std::cout << "Failed to load" << name << std::endl;
		return;
	}

	numVertices = RAW_WIDTH * RAW_HEIGHT;
	numIndices = (RAW_WIDTH - 1) * (RAW_HEIGHT - 1) * 6;
	vertices = new glm::vec3[numVertices];
	textureCoords = new glm::vec2[numVertices];
	indices = new GLuint[numIndices];

	unsigned char* data = new unsigned char[numVertices];
	file.read((char*)data, numVertices * sizeof(unsigned char));
	file.close();

	for (int x = 0; x < RAW_WIDTH; ++x) {
		for (int z = 0; z < RAW_HEIGHT; ++z) {
			int offset = (x * RAW_WIDTH) + z;

			vertices[offset] = glm::vec3(x * HEIGHTMAP_X, data[offset] * HEIGHTMAP_Y, z * HEIGHTMAP_Z);
			textureCoords[offset] = glm::vec2(x * HEIGHTMAP_TEX_X, z * HEIGHTMAP_TEX_Z);
		}
	}

	delete data;

	numIndices = 0;

	for (int x = 0; x < RAW_WIDTH - 1; ++x) {
		for (int z = 0; z < RAW_HEIGHT - 1; ++z) {
			int a = x * RAW_WIDTH + z;
			int b = (x + 1) * RAW_WIDTH + z;
			int c = (x + 1) * RAW_WIDTH + (z + 1);
			int d = x * RAW_WIDTH + (z + 1);

			indices[numIndices++] = c;
			indices[numIndices++] = b;
			indices[numIndices++] = a;

			indices[numIndices++] = a;
			indices[numIndices++] = d;
			indices[numIndices++] = c;
		}
	}

	GenerateNormals();
	GenerateTangents();
	BufferData();

}

HeightMap::HeightMap() {
	numVertices = RAW_WIDTH * RAW_HEIGHT;
	numIndices = (RAW_WIDTH - 1) * (RAW_HEIGHT - 1) * 6;
	vertices = new glm::vec3[numVertices];
	textureCoords = new glm::vec2[numVertices];
	indices = new GLuint[numIndices];

	for (int x = 0; x < RAW_WIDTH; ++x) {
		for (int z = 0; z < RAW_HEIGHT; ++z) {
			int offset = (x * RAW_WIDTH) + z;

			vertices[offset] = glm::vec3(x * HEIGHTMAP_X, 0, z * HEIGHTMAP_Z);
			textureCoords[offset] = glm::vec2(x * HEIGHTMAP_TEX_X, z * HEIGHTMAP_TEX_Z);
		}
	}

	numIndices = 0;

	for (int x = 0; x < RAW_WIDTH - 1; ++x) {
		for (int z = 0; z < RAW_HEIGHT - 1; ++z) {
			int a = x * RAW_WIDTH + z;
			int b = (x + 1) * RAW_WIDTH + z;
			int c = (x + 1) * RAW_WIDTH + (z + 1);
			int d = x * RAW_WIDTH + (z + 1);

			indices[numIndices++] = c;
			indices[numIndices++] = b;
			indices[numIndices++] = a;

			indices[numIndices++] = a;
			indices[numIndices++] = d;
			indices[numIndices++] = c;
		}
	}

	GenerateNormals();
	GenerateTangents();
	BufferData();
}

float HeightMap::GetHeightAtPosition(float x, float z) {
	int row = int(round(x / HEIGHTMAP_X)) % RAW_WIDTH;
	int col = int(round(z / HEIGHTMAP_Z)) % RAW_HEIGHT;

	if (row >= RAW_WIDTH || col >= RAW_HEIGHT) {
		return 0.0f;
	}
	else {
		return vertices[row + col].y;
	}

}