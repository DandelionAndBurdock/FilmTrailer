#include "GerstnerWaves.h"

#include <sstream>
#include <iostream>

#include "ShaderManager.h"

namespace WaveConstants {
	// Grid mesh constants
	const int	RAW_WIDTH = 257;
	const int	RAW_HEIGHT = 257;
	const float HEIGHTMAP_X = 16.0f;
	const float HEIGHTMAP_Z = 16.0f;
	const float HEIGHTMAP_Y = 2.25f;
	const float HEIGHTMAP_TEX_X = 1.0f / 16.0f;
	const float HEIGHTMAP_TEX_Z = 1.0f / 16.0f;

	// Restrict max waves to ease joining up with GLSL
	const int MAX_WAVES = 8;
};

using namespace WaveConstants;

GerstnerWaves::GerstnerWaves() 
{
	numVertices = RAW_WIDTH * RAW_HEIGHT;
	numIndices = (RAW_WIDTH - 1)*(RAW_HEIGHT - 1) * 6;
	vertices = new glm::vec3[numVertices];
	textureCoords = new glm::vec2[numVertices];
	indices = new GLuint[numIndices];


	for (int x = 0; x < RAW_WIDTH; ++x) {
		for (int z = 0; z < RAW_HEIGHT; ++z) {
			int offset = (x * RAW_WIDTH) + z;

			vertices[offset] = glm::vec3(x * HEIGHTMAP_X, 0.0, z * HEIGHTMAP_Z);

			textureCoords[offset] = glm::vec2(x * HEIGHTMAP_TEX_X, z * HEIGHTMAP_TEX_Z);
		}
	}
	numIndices = 0;

	for (int x = 0; x < RAW_WIDTH - 1; ++x){
		for (int z = 0; z < RAW_HEIGHT - 1; ++z){
			int a = (x     * (RAW_WIDTH)) + z;
			int b = ((x + 1) * (RAW_WIDTH)) + z;
			int c = ((x + 1) * (RAW_WIDTH)) + (z + 1);
			int d = (x     * (RAW_WIDTH)) + (z + 1);

			indices[numIndices++] = c;
			indices[numIndices++] = b;
			indices[numIndices++] = a;
					
			indices[numIndices++] = a;
			indices[numIndices++] = d;
			indices[numIndices++] = c;
		}
	}
	GenerateNormals();
	GenerateTangents();
	BufferData();


	waves.push_back(Wave(glm::vec2(1.0, 0.0), 128.0f, 0.05f, 10.0f));
	waves.push_back(Wave(glm::vec2(1.0, 1.0), 200.0f, 0.025f, 10.0f));
	waves.push_back(Wave(glm::vec2(0.0, 1.0), 256.0f, 0.1f, 5.0f));

}


GerstnerWaves::~GerstnerWaves()
{
}

void GerstnerWaves::SendDataToShader()
{
	int numWaves = GetNumWaves();
	for (int i = 0; i < numWaves; ++i)
	{
		glUniform2fv(GetUniformLocation("direction", i), 1, (float*)&waves[i].direction);
		glUniform2fv(GetUniformLocation("wavevector", i), 1, (float*)&waves[i].wavevector);
		glUniform1f(GetUniformLocation("wavelength", i), waves[i].wavelength);
		glUniform1f(GetUniformLocation("angularFreq", i), 2 * PI * waves[i].frequency);
		glUniform1f(GetUniformLocation("phase", i), waves[i].phase);
		glUniform1f(GetUniformLocation("amplitude", i), waves[i].amplitude);
		float Q = qFactor / (waves[i].wavevector.length() * waves[i].amplitude * numWaves); //TODO: Caclulate this once on construction. Amplitude cancels?
		glUniform1f(GetUniformLocation("qFactor", i), Q);
	}

	// Make sure everything else is intialised to 0
	for (int i = waves.size(); i < numWaves; ++i)
	{
		glUniform2f(GetUniformLocation("direction", i), 0.0f, 0.0f);
		glUniform2f(GetUniformLocation("wavevector", i), 0.0f, 0.0f);
		glUniform1f(GetUniformLocation("wavelength", i), 0.0f);
		glUniform1f(GetUniformLocation("angularFreq", i), 0.0f);
		glUniform1f(GetUniformLocation("amplitude", i), 0.0f);
		glUniform1f(GetUniformLocation("phase", i), 0.0f);
		glUniform1f(GetUniformLocation("qFactor", i), 0.0f);
	}
		
}

void GerstnerWaves::Update(float time)
{
	SHADER_MANAGER->SetUniform("GerstnerShader", "time", time);
}


int GerstnerWaves::GetNumWaves() 
{
	if (waves.size() > WaveConstants::MAX_WAVES)
		return WaveConstants::MAX_WAVES;
	else
		return waves.size();
}

// Returns the uniform corresponding to Waves[wavenumber].variable . Prints error message if failed to find.
GLuint GerstnerWaves::GetUniformLocation(const char* variable, GLint wavenumber)
{
	const std::string start = "Waves[";
	const std::string end = "].";
	std::stringstream location;
	location << start << wavenumber << end;
	GLuint uniformLocation = glGetUniformLocation(SHADER_MANAGER->GetShader("GerstnerShader")->GetProgram(), 
		std::string(location.str() + std::string(variable)).c_str());
	if (uniformLocation < 0)
		std::cout << "Terrible ERROR!!! Can't find: " << std::string(location.str() + std::string(variable)) << std::endl;

	return uniformLocation;
}

void GerstnerWaves::Draw()
{
	SendDataToShader();
	Mesh::Draw();
}