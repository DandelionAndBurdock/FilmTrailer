// https://www.youtube.com/watch?v=LyoSSoYyfVU
#version 150

uniform sampler2D diffuseTex;


in Vertex {
	vec2 texCoord;
} IN;


out vec4 gl_FragColor;


void main(void){
	vec4 colour = texture(diffuseTex, IN.texCoord);
	// Get brightness of colour (can be done in several different ways e.g. Luma Conversion, average)
	float brightness = length(colour.rgb);
	// By muliplying by brightness anything dark gets darker (brightness < 1) anything bright (brightness > 1) gets lighter
	colour.rgb *= brightness;
}
