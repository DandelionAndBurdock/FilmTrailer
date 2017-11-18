#version 330 core

in vec3 position;
in vec2 texCoordIn;
 
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

out vec4 clipSpacePos;
out vec2 texCoord;

const float TILING = 6.0; // Tile the dudv Map
void main(void)
{
	clipSpacePos = (projMatrix * viewMatrix * modelMatrix) * vec4(position, 1.0);
	gl_Position = clipSpacePos;
	//texCoord = texCoordIn;
	//texCoord = vec2(position.x / 2.0 + 0.5, position.y / 2.0 + 0.5) * TILING;
	texCoord = vec2(position.x / 2.0 + 0.5, position.y / 2.0 + 0.5);
}