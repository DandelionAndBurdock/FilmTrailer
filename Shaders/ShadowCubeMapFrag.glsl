#version 330 core

in vec4 fragWorldPos;

uniform vec3 lightWorldPos;
uniform float farPlane;

void main()
{
	float lightDistance = length(fragWorldPos.xyz - lightWorldPos);

    // Linearise the depth (just to simplify shadow calculations)
    gl_FragDepth = lightDistance / farPlane;
}  