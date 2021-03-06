#include "Scene.h"

#include "SceneNode.h"
#include "HeightMap.h"

#include "ParticleSystem.h" // Rename firework system
#include "ParticleEmitter.h"
#include "Lightning.h"
#include "ParticleManager.h"
#include "Light.h"

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


void Scene::UpdateEffects(GLfloat msec, const glm::vec3& cameraPos) {
	if (fireworks) {
		fireworks->UpdateParticles(msec);
	}
	if (emitterOn && emitter) {
		emitter->Update(msec);
	}

	if (lightning) {
		lightning->Update(msec);
	}

	if (particles) {
		particles->Update(msec, cameraPos);
	}
}
void Scene::DrawEffects(const glm::mat4& viewProj, const glm::vec3& cameraPos) {
	if (fireworks) {
		fireworks->Render(viewProj, cameraPos);
	}

	if (emitterOn && emitter) {
		emitter->Draw();
	}

	if (lightning && lightning->ShouldFire()) {
		lightning->Draw(viewProj, cameraPos);
	}

	if (particles) {
		particles->Render();
	}
}


void Scene::SetEmitter(ParticleEmitter* particleEmitter) {
	emitter = particleEmitter;
	emitter->SetParticleSize(8.0f);
	emitter->SetParticleVariance(1.0f);
	emitter->SetLaunchParticles(16.0f);
	emitter->SetParticleLifetime(2000.0f);
	emitter->SetParticleSpeed(0.03f);
	emitter->SetSpawnPoint(glm::vec3(1150.0f, 0.0f, 800.0f));
}

void Scene::AddLight(Light* light) { 
	permanentLights.push_back(light); 
	terrain->AddChild(light);
}