#version 150

uniform sampler2D grassTex;

// Input to fragment shader
in Vertex {
	vec2 texCoord;
} IN;

out vec4 gl_FragColor;

void main(void){
	gl_FragColor = texture(grassTex, IN.texCoord);
}