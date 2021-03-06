#version 150
uniform sampler2D diffuseTex;


in Vertex	{
	vec4 	colour;
	vec2 	texCoord;
	vec3 	tangent;
	vec3 	worldPos;
	vec3 	normal;
} IN;

out vec4 gl_FragColor;

void main(void){
	//gl_FragColor = texture(diffuseTex, IN.texCoord);
	gl_FragColor = vec4(0.0, IN.texCoord.x / 2.0, IN.texCoord.y / 4.0, 1.0);
}