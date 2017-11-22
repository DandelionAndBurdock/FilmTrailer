#include "Scene.h"

#include "SceneNode.h"
#include "HeightMap.h"

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


