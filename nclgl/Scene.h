#pragma once

#include "../GLEW/include/GL/glew.h"

class SceneNode;
class HeightMap;
class OBJMesh;
class FireworkSystem;
class ParticleManager;
class DirectionalLight;
class Lightning;
class Spotlight;
class Grass;
class Sun;
class Water;
class ParticleEmitter;

#include <vector>

#include "../glm/mat4x4.hpp"
#include "../glm/vec3.hpp"

class Scene
{
public:
	Scene(SceneNode* masterRoot);
	~Scene();

	SceneNode* GetRoot() { return root; }
	void SetTerrain(SceneNode* heightMap);

	void SetCubeMap(GLuint cubeMap) { this->cubeMap = cubeMap; }
	GLuint GetCubeMap() { return cubeMap; }

	void UpdateEffects(GLfloat msec);
	void DrawEffects(const glm::mat4& viewProj, const glm::vec3 cameraPos);

	void SetFireworks(FireworkSystem* firework) { fireworks = firework; }
	void SetEmitter(ParticleEmitter* particleEmitter);

protected:
SceneNode* root = nullptr;
SceneNode* terrain = nullptr;

GLuint cubeMap = 0;

// Effects
FireworkSystem* fireworks = nullptr;
ParticleEmitter* emitter = nullptr;
};

