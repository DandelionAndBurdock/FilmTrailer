#include "Light.h"

#include "OBJMesh.h"

#include "../glm/gtx/transform.hpp"	

Mesh* Light::cube = nullptr;

bool Light::CreateLightMesh() {

	OBJMesh* m = new OBJMesh;
	if (m->LoadOBJMesh(MESHDIR"cube.obj")) {
		cube = m;
		return true;
	}
	return false;

}

void Light::UpdateTransform(){ 
	transform = glm::translate(position); 
}