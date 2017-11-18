#version 150
uniform sampler2D diffuseTex;

in Vertex {
	vec2 texCoord;
} IN;

//in Vertex	{
//	vec4 	colour;
//	vec2 	texCoord;
//	vec3 	normal;
//	vec3 	tangent;
//	vec3 	worldPos;
//} IN;

out vec4 gl_FragColor;

void main(void){
	gl_FragColor = texture(diffuseTex, IN.texCoord);
}