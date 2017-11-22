#pragma once

#include "Mesh.h"
#include <vector>

#include "../GLEW/include/GL/glew.h"
#include "../glm/vec2.hpp"

namespace WaveConstants
{
	struct Wave
	{
		glm::vec2 wavevector;	 // Direction of wave travel in the xz-plane
		glm::vec2 direction;	 // Direction of wave travel in the xz-plane
		float wavelength;	 // Distance between consecutive peaks
		float frequency;	 // Frequency [Hz]
		float phase;		 // Offset of wave
		float amplitude;	 // Maximum displacement from equilibrium position;

		Wave(glm::vec2 direct, float wlength, float freq, float amp, float phi = 0.0f) :
			wavelength(wlength), frequency(freq),
			phase(phi), amplitude(amp)
		{
			glm::normalize(direct);
			direction = direct;
			wavevector = direction * (2 * PI / wavelength);
		}
	};

	

}

class GerstnerWaves : public Mesh
{
public:
	GerstnerWaves();
	~GerstnerWaves();

	void SendDataToShader();
	void Update(float msec);
	void Draw();

protected:
	
	int GetNumWaves(); 
	std::vector<WaveConstants::Wave> waves;

	int qFactor = 1; // Between 0 and 1 determines steepness of the wave


private:
	// Helper function
	GLuint GetUniformLocation(const char* variable, GLint wavenumber);
};

