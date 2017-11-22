// Walt Ritscher https://stackoverflow.com/questions/944713/help-with-pixel-shader-effect-for-brightness-and-contrast
#version 150

uniform sampler2D diffuseTex;


in Vertex {
	vec2 texCoord;
} IN;

uniform float contrast;

out vec4 gl_FragColor;

void main(void){
	gl_FragColor = texture(diffuseTex, IN.texCoord);
	// Change colours from range [0. 1] to [-0.5, 0.5]
	gl_FragColor.rgb -= vec3(0.5);
	// Scale bright colours to be brighter and darker colours darker
	gl_FragColor.rgb *= (1.0 + contrast);
	// Return to a valid colour range
	gl_FragColor.rgb += vec3(0.5);
}
