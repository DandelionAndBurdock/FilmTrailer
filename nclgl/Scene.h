#pragma once

#include "../GLEW/include/GL/glew.h"

class SceneNode;
class HeightMap;
class OBJMesh;
class ParticleSystem;
class ParticleManager;
class DirectionalLight;
class Lightning;
class Spotlight;
class Grass;
class Sun;
class Water;

#include <vector>

class Scene
{
public:
	Scene(SceneNode* masterRoot);
	~Scene();

	SceneNode* GetRoot() { return root; }
	void SetTerrain(SceneNode* heightMap);

	void SetCubeMap(GLuint cubeMap) { this->cubeMap = cubeMap; }
	GLuint GetCubeMap() { return cubeMap; }

protected:
SceneNode* root;
SceneNode* terrain;

GLuint cubeMap;
};

