#version 150 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

in vec3 position;
in vec4 colour;
in vec3 normal;
in vec2 texCoord;

const int MAX_WAVES = 8;

struct Wave
{
		vec2 direction;		 // Direction of wave
        vec2 wavevector;	 // Points in direction of wave travel with magnitude (2 * PI / lambda)
        float angularFreq;   // Angular frequency (omega)
        float phase;		 // Offset of wave
		float amplitude;	 // Maximum displacement from equilibrium position;
		float qFactor;		// Determines steepness of the wave
};

uniform float time;			 // Time elapsed since creation of the water body

// Limited to a maximum number of 8 waves in this implementation
uniform Wave Waves[MAX_WAVES];

out Vertex {
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
	vec3 modelPos;
} OUT;


void main(void){
	// Calculate vertex positions
	vec3 wavePosition = vec3(modelMatrix * vec4(position, 1.0));
	for (int i = 0; i < MAX_WAVES; ++i)
	{
		wavePosition.x += Waves[i].qFactor * Waves[i].amplitude * Waves[i].wavevector.x * cos(dot(Waves[i].wavevector, position.xz) - Waves[i].angularFreq * time + Waves[i].phase); //TODO: Precalculate sine and cosine values
		wavePosition.z += Waves[i].qFactor * Waves[i].amplitude * Waves[i].wavevector.y * cos(dot(Waves[i].wavevector, position.xz) - Waves[i].angularFreq * time + Waves[i].phase);
		wavePosition.y += Waves[i].amplitude * sin(dot(Waves[i].wavevector, position.xz) - Waves[i].angularFreq * time + Waves[i].phase);
		wavePosition.y += sin(dot(Waves[i].wavevector, position.xz));
	}
		
	// Calculate normals
	vec3 waveNormal = vec3(0.0, 1.0, 0.0);
		for (int i = 0; i < MAX_WAVES; ++i)
	{
		float kMag = length(Waves[i].wavevector);// TODO: Precalculate this
		waveNormal.x -= Waves[i].wavevector.x * Waves[i].amplitude * cos(dot(Waves[i].wavevector, position.xz) - Waves[i].angularFreq * time + Waves[i].phase); 
		waveNormal.z -= Waves[i].wavevector.y * Waves[i].amplitude * cos(dot(Waves[i].wavevector, position.xz) - Waves[i].angularFreq * time + Waves[i].phase);
		waveNormal.y -= Waves[i].amplitude * kMag * Waves[i].qFactor * sin(dot(Waves[i].wavevector, position.xz) - Waves[i].angularFreq * time + Waves[i].phase);
	}

	vec3 waveTangent = vec3(0.0, 0.0, 1.0);
		for (int i = 0; i < MAX_WAVES; ++i)
		{
			float kMag = length(Waves[i].wavevector);// TODO: Precalculate this
			waveTangent.x -= Waves[i].wavevector.x * Waves[i].direction.y * Waves[i].amplitude * Waves[i].qFactor * sin(dot(Waves[i].wavevector, position.xz) - Waves[i].angularFreq * time + Waves[i].phase);
			waveTangent.z -= Waves[i].wavevector.y * Waves[i].direction.y * Waves[i].amplitude * Waves[i].qFactor * sin(dot(Waves[i].wavevector, position.xz) - Waves[i].angularFreq * time + Waves[i].phase);
			waveTangent.y -= Waves[i].amplitude * Waves[i].wavevector.y * cos(dot(Waves[i].wavevector, position.xz) - Waves[i].angularFreq * time + Waves[i].phase);
		}
		vec3 waveBinormal = vec3(1.0, 0.0, 0.0); //TODO: This is the same as binormal can save a lot of multiplies
		for (int i = 0; i < MAX_WAVES; ++i)
		{
			waveBinormal.z -= Waves[i].wavevector.x * Waves[i].direction.y * Waves[i].amplitude * Waves[i].qFactor * sin(dot(Waves[i].wavevector, position.xz) - Waves[i].angularFreq * time + Waves[i].phase);
			waveBinormal.x -= Waves[i].wavevector.y * Waves[i].direction.y * Waves[i].amplitude * Waves[i].qFactor * sin(dot(Waves[i].wavevector, position.xz) - Waves[i].angularFreq * time + Waves[i].phase);
			waveBinormal.y -= Waves[i].amplitude * Waves[i].wavevector.x * cos(dot(Waves[i].wavevector, position.xz) - Waves[i].angularFreq * time + Waves[i].phase);
		}

	OUT.colour = colour;
	OUT.texCoord = vec4(texCoord, 0.0, 1.0).xy;
	OUT.modelPos = wavePosition;
	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	OUT.normal = normalize(normalMatrix * normalize(waveNormal)); 
	OUT.tangent = normalize(normalMatrix * normalize(waveTangent));
	OUT.binormal = normalize(normalMatrix * normalize(waveBinormal));
	OUT.worldPos = (modelMatrix * vec4(position, 1)).xyz;
	gl_Position = (projMatrix * viewMatrix * modelMatrix) * vec4(wavePosition, 1.0);
}