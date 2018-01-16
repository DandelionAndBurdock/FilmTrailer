#version 330 core
in vec4 FragPos;

uniform vec3 lightPos;
const float farPlane = 25.0f;

void main()
{
    float lightDistance = length(FragPos.xyz - lightPos);
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / farPlane;
    
    // write this as modified depth
    gl_FragDepth = lightDistance;
	gl_FragDepth = 0.0;
}