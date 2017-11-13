#version 150
uniform sampler2D text;
uniform vec3 textColour;

in Vertex {
	vec2 texCoord;
} IN;

out vec4 gl_FragColor;

void main(void){
	// Since text is a grayscale texture we can sample to get the alpha value
	// Black parts of the texture will be transparent
	float alpha = texture(text, IN.texCoord).r;
	gl_FragColor = vec4(textColour, alpha);
}