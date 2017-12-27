#include "ShaderArt.h"

#include "Mesh.h"
#include "Shader.h"

ShaderArt::ShaderArt(GLfloat sWidth, GLfloat sHeight)
{
	shader = new Shader(SHADERDIR"PassThroughVertex.glsl", SHADERDIR"LavaLamp.frag");
	if (!shader->WasLoaded()) {
		__debugbreak();
	}
	else {
		shader->LinkProgram();
	}

	quad = Mesh::GenerateQuad();

	screenHeight = sHeight;
	screenWidth = sWidth;
}


ShaderArt::~ShaderArt()
{
	if (shader) {
		delete shader;
	}

	if (quad) {
		delete quad;
	}
}

void ShaderArt::Draw() {
	shader->Use();
	shader->SetUniform("resolution", glm::vec2(screenWidth, screenHeight));
	shader->SetUniform("time", time);
	quad->Draw();
}

void ShaderArt::Update(GLfloat timeSec) {
	time = timeSec;
}