#pragma once

#include <string>


#include "Mesh.h"
#include "SceneNode.h"

#define RAW_WIDTH 257
#define RAW_HEIGHT 257

#define HEIGHTMAP_X 16.0f
#define HEIGHTMAP_Z 16.0f
#define HEIGHTMAP_Y 1.25f
#define HEIGHTMAP_TEX_X 1.0f / 16.0f
#define HEIGHTMAP_TEX_Z 1.0f / 16.0f


class HeightMap : public Mesh, public SceneNode
{
public:
	HeightMap(std::string name);
	~HeightMap() {};
};