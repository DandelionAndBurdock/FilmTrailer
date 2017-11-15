#include "Lightning.h"


#include "ShaderManager.h"
#include "common.h" //TODO: Remove
#include "RandomNumberGenerator.h"
Lightning::Lightning(const glm::vec3& from, const glm::vec3& to)
{
	motherFrom = from;
	motherTo = to;

	Generate(from, to);
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &lightningVAO);
	glBindVertexArray(lightningVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, bolt.size() * sizeof(Point),
		&bolt[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0); // Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), 0);
	glEnableVertexAttribArray(1); // Thickness
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)sizeof(glm::vec3));
	glDeleteBuffers(1, &VBO);

	SHADER_MANAGER->AddShader("Lightning", SHADERDIR"LightningVertex.glsl", SHADERDIR"LightningFrag.glsl", SHADERDIR"LightningGeom.glsl");

}


Lightning::~Lightning()
{
	glDeleteVertexArrays(1, &lightningVAO);
}

void Lightning::Generate(const glm::vec3& from, const glm::vec3& to) {
	AddBranch(from, to, BASE_THICKNESS, NUM_POINTS);

}

void Lightning::AddBranch(const glm::vec3& from, const glm::vec3& to, float thickness, int numPoints) {
	if (thickness < 0.2) //TODO: Remove magic numbers
		return;
	// Add vertices
	std::vector<glm::vec3> straightLine = Divide(from, to, numPoints);
	AddJitter(straightLine);
	for (const auto& vertex : straightLine) {
		bolt.push_back(Point(vertex, thickness));
	}
	drawVertices.push_back(straightLine.size());

	// Recursively add more forks
	std::vector<int> forkPoints = GetForkPoints(straightLine.size(), BASE_NEW_BRANCH);
	for(int i : forkPoints){
		glm::vec3 dest = GetForkDestination(from, to, straightLine[i]);
		AddBranch(straightLine[i], dest, thickness * 0.75f, numPoints / 2);
	}

}

std::vector<int> Lightning::GetForkPoints(int size, float prob) {
	if (prob < 0 || prob > 1) {
		return std::vector<int>();
	}
	std::vector<int> points;
	for (int i = 0; i < size; ++i) {
		if (RNG->GetRandFloat() < prob) {
			points.push_back(i);
			i += 5; // Don't fork from adjacent points
		}
	}
	return points;
}
glm::vec3  Lightning::GetForkDestination(const glm::vec3& parentFrom, const glm::vec3& parentTo, glm::vec3 branchFrom) {
	// Fork to the left or right : To decide we can find the normal to the plane of the parent //TODO:Think: Am I right here? Not sure
	glm::vec3 normal = glm::normalize(glm::cross(branchFrom - parentFrom, parentTo - parentFrom));
	if (normal.y > 0) {
		normal.y *= -1;
	}
	// Keep it moving in roughly the right direction
	glm::vec3 direction = normal + glm::normalize(motherTo - parentFrom);

	// Want to create shorter branches the closer we are to the end of the bolt
	float ratio = branchFrom.y / (motherTo.y + branchFrom.y); //TODO: Div by 0?
	float parentLength = glm::length(parentFrom - parentTo);
	float length = ratio * RNG->GetRandFloat(0.25f * parentLength, 0.5f * parentLength);

	glm::vec3 destination = branchFrom + length * direction;

	// Add a bit of noise in x-z plane
	destination.x += RNG->GetRandInt(-5, 5);
	destination.z += RNG->GetRandInt(-5, 5);
	destination.y = max(motherTo.y, destination.y);

	return destination;
}

std::vector<glm::vec3> Lightning::Divide(const glm::vec3& from, const glm::vec3& to, int numSubDivisions) {
	std::vector<glm::vec3> vertices;
	glm::vec3 vertex = from;
	glm::vec3 line = to - from;
	vertices.push_back(vertex);
	for (int i = 0; i < numSubDivisions; ++i) {
		vertex += line / float(numSubDivisions);
		vertices.push_back(vertex);
	}

	return vertices;
}

void Lightning::Draw(const glm::mat4& viewProj, const glm::vec3 cameraPos) {
	SHADER_MANAGER->SetShader("Lightning");
	SHADER_MANAGER->SetUniform("Lightning", "cameraPos", cameraPos);
	SHADER_MANAGER->SetUniform("Lightning", "viewProjMatrix", viewProj);
	//
	////glDisable(GL_CULL_FACE);
	glBindVertexArray(lightningVAO);
	int startIndex = 0;
	for (const auto& numVerts : drawVertices) {
		glDrawArrays(GL_LINE_STRIP, startIndex, numVerts);
		startIndex += numVerts;
	}


	
}

void Lightning::AddJitter(std::vector<glm::vec3>& verts) {
	// Don't jitter first vertex so that branches match up
	for (int i = 1; i < verts.size(); ++i) {
		verts[i].x += RNG->GetRandInt(-3, 3);
		verts[i].y += RNG->GetRandInt(-3, 3);
		verts[i].z += RNG->GetRandInt(-3, 3);
	}
}
