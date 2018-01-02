#version 150

uniform sampler2D originalScene;
uniform sampler2D smash;
uniform sampler2D background;

uniform float lowerThreshold;
uniform float upperThreshold;
in Vertex {
	vec2 transformedTexCoord;
	vec2 texCoord;
} IN;

uniform float contrast;

out vec4 gl_FragColor;

void main(void){
	// Check have valid texCoords 
	if (IN.transformedTexCoord.x > 1) {
		discard;
	}
	if (IN.transformedTexCoord.x < 0) {
		discard;
	}
	if (IN.transformedTexCoord.y > 1) {
		discard;
	}
	if (IN.transformedTexCoord.y < 0) {
		discard;
	}

	// Is black line?
	if (texture(smash, IN.transformedTexCoord).x < 0.1) {
		gl_FragColor = texture(background, IN.transformedTexCoord);
	}
	else {
		// Is non-piece?
		float pieceColour = texture(smash, IN.transformedTexCoord).x;
		if (pieceColour < lowerThreshold || pieceColour > upperThreshold) {
			gl_FragColor = texture(background, IN.transformedTexCoord);
		}
		else { // Is piece
			gl_FragColor = texture(originalScene, IN.transformedTexCoord);
		}
	}
}
