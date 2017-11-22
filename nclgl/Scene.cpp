#include "Scene.h"

#include "SceneNode.h"
#include "HeightMap.h"

#include "ParticleSystem.h" // Rename firework system
#include "ParticleEmitter.h"
Scene::Scene(SceneNode* masterRoot)
{
	root = new SceneNode();
	masterRoot->AddChild(root);
	terrain = nullptr;
}


Scene::~Scene()
{
	delete root;
	delete terrain;
}

void Scene::SetTerrain(SceneNode* heightMap) {
	if (terrain) {
		delete terrain;
	}
	terrain = heightMap;
	root->AddChild(terrain);
}


void Scene::UpdateEffects(GLfloat msec) {
	if (fireworks) {
		fireworks->UpdateParticles(msec);
	}
	if (emitter) {
		emitter->Update(msec);
	}
}
void Scene::DrawEffects(const glm::mat4& viewProj, const glm::vec3 cameraPos) {
	if (fireworks) {
		fireworks->Render(viewProj, cameraPos);
	}

	if (emitter) {
		emitter->Draw();
	}
}


void Scene::SetEmitter(ParticleEmitter* particleEmitter) {
	emitter = particleEmitter;
	emitter->SetParticleSize(8.0f);
	emitter->SetParticleVariance(1.0f);
	emitter->SetLaunchParticles(16.0f);
	emitter->SetParticleLifetime(2000.0f);
	emitter->SetParticleSpeed(0.1f);
	emitter->SetSpawnPoint(glm::vec3(0.0f));
}