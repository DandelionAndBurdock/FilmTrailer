#version 150
uniform sampler2D diffuseTex;

in Vertex {
	float height;
	vec2 texCoord;
} IN;

out vec4 gl_FragColor;

void main(void){
vec4 white = vec4(1.0, 1.0, 1.0, 1.0);
	gl_FragColor = mix(texture(diffuseTex, IN.texCoord), white, IN.height);
}