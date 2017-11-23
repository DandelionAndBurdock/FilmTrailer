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



glm::vec4 Light::GetPosition() const {
	//if (parent) {
	//	glm::mat4 temp = parent->GetWorldTransform();
	//	return parent->GetWorldTransform() * transform * glm::vec4(position, 1.0);
	//}
	//else {
		return glm::vec4(position, 1.0);
	//}
	
}