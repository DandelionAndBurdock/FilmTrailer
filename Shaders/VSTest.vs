#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

out vec3 normalWorld;
out vec3 fragPositionWorld;
out vec2 textureCoord;

uniform mat4 modelMatrix; //Local object coordinates to camera coordinates
uniform mat4 viewMatrix; //Normalised coordinates to window coordintes
uniform mat4 projectionMatrix; //Camera coordinates to normalised coordinates

void main()
{
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
	fragPositionWorld = vec3(modelMatrix * vec4(position, 1.0f));
	//normalWorld = mat3(transpose(inverse(modelMatrix))) * normal;
	normalWorld = normal;
	textureCoord = texCoord;

}