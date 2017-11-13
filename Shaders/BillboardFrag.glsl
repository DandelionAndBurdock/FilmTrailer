#version 150 core

uniform sampler2D diffuseTex;

in vec2 texCoord;
in vec4 fragColour;

out vec4 gl_FragColor;

void main()
{
	gl_FragColor = fragColour * texture2D(diffuseTex, texCoord);
}