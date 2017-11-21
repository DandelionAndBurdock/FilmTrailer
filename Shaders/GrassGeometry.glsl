// Creates grass cluster
// Based on: http://www.mbsoftworks.sk/index.php?page=tutorials&series=1&tutorial=32
#version 150 core

layout (points) in; 				// Single point specifies position of glass cluster  in model space
layout (triangle_strip) out;		// Build quads from triangle strips
layout (max_vertices = 12) out;		// Three quads of 4 vertices

uniform float time; 	// Time since started simulating grass in seconds

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

// Ouput to fragment shader
out Vertex {
	vec2 texCoord;
} OUT;

// Random Function relies on rapid modulation of high frequency sine wave
//http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
highp float RandZeroToOne(vec2 seed)
{
    highp float a = 12.9898;
    highp float b = 78.233;
    highp float c = 43758.5453;
    highp float dt= dot(seed.xy ,vec2(a,b));
    highp float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}

// Returns rotation matrix for an angle theta about axis 
// using Equation from Page 3 of Vertex Transformation Tutorial
mat3 RotationMatrix(vec3 axis, float angle) 
{ 
    float s = sin(angle); 
    float c = cos(angle); 
    float oc = 1.0 - c; 
     
    return mat3(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c          ); 
}


int randomInt(int min, int max, vec2 seed)
{
	return int(float(min)+RandZeroToOne(seed)*float(max-min));
}


void main() {
	// Some useful constants
	const float PI = 3.14159265359;
	const vec3 Y_AXIS = vec3(0.0, 1.0, 0.0);
	const float omega = 0.7f; // Angular Frequency for grass waving animation
	const float amplitude = 0.1f; // Modulating amplitude for the grass waving animation
	
// Will position three overlapping quads: When viewed from above 
// One horizontal and two orthogonal quads at 
// 45 degrees to the horizontal to form a "star" pattern
	const int NUM_QUADS = 3;
   vec3 quadAxes[NUM_QUADS];
   quadAxes[0] = vec3(1.0, 0.0, 0.0); 							// Horizontal
   quadAxes[1] = vec3(+cos(PI / 4.0), 0.0f, +sin(PI / 4.0));	// Positive diagonal
   quadAxes[2] = vec3(+cos(PI / 4.0), 0.0f, -sin(PI / 4.0));	// Negative diagonal
   
      // Grass characteristics make uniform
   	float grassClusterSize = 5.0; // Size of a quad
	vec3 grassClusterPosition = gl_in[0].gl_Position.xyz; 	// Cluster position in model space
	float halfQuadLength = grassClusterSize * 0.5; 
   
   // Wind strength
   //Note that wind will only affect the top vertices of the quad and has no effect on the bottom vertices
   //TODO: Make uniform, remove magic numbers
   	float windStrength = 4.0;
	vec3 windDirection = normalize(vec3(1.0, 0.0, 1.0));
	// Wind strength at this cluster position
    float localWindStrength = windStrength * (0.5f+sin(dot(grassClusterPosition, windDirection)/ 30.0 + time*1.2f)); 
	// Put some asymmetry in the direction so it looks less artificial
	if (localWindStrength < 0)
		windStrength *= 0.2;
	else
		windStrength *= 0.3;
		
   // Model-View-Projection Matrix
   mat4 mvp = projMatrix * viewMatrix * modelMatrix;

   // For each axis produce one quad using four corner vertices
   for (int i = 0; i < NUM_QUADS; ++i){
   
   // Grass is animated by rotating the star of quads around the y-axis
	vec3 rotatedAxisDir = RotationMatrix(Y_AXIS, amplitude * sin(omega * time)) * quadAxes[i];
	rotatedAxisDir = quadAxes[i];
	// Randomise grass height 
   float grassClusterHeight = 3.5 + RandZeroToOne(grassClusterPosition.xz) * 2.0;

		 // Texture contains 4 possible grass clusters choose one are random to represent this quad //TODO: Remove magic numbers
		// Note seed with grassPosition so that texture remains consistent from frame to frame 
		vec2 seed = grassClusterPosition.xz * float(i);
		int textureChoice = randomInt(0, 3, seed); // Which texture to sample
		
		// Grass texure is 1028 x 128 pixels with each different cluster image taking 1/4 of the length
		float texCoordBeginX = float(textureChoice)*0.25f;
		float texCoordEndX = texCoordBeginX + 0.25f;
		
		// Top Left Vertex
		vec3 topLeft = grassClusterPosition - rotatedAxisDir * halfQuadLength +windDirection * localWindStrength;
		topLeft.y += grassClusterHeight;
		gl_Position = mvp * vec4(topLeft, 1.0);
		OUT.texCoord = vec2(texCoordBeginX, 1.0);
		EmitVertex();
		
		// Bottom Left Vertex
		vec3 bottomLeft = grassClusterPosition - rotatedAxisDir * halfQuadLength;
		gl_Position = mvp * vec4(bottomLeft, 1.0);
		OUT.texCoord = vec2(texCoordBeginX, 0.0);
		EmitVertex();
		
		// Top Right Vertex
		vec3 topRight = grassClusterPosition + rotatedAxisDir * halfQuadLength +windDirection * localWindStrength;
		topRight.y += grassClusterHeight;
		gl_Position = mvp * vec4(topRight, 1.0);
		OUT.texCoord = vec2(texCoordEndX, 1.0);
		EmitVertex();
		
		// Bottom Right Vertex
		vec3 bottomRight = grassClusterPosition + rotatedAxisDir * halfQuadLength;
		gl_Position = mvp * vec4(bottomRight, 1.0);
		OUT.texCoord = vec2(texCoordEndX, 0.0);
		EmitVertex();
		
		EndPrimitive();
   }

}