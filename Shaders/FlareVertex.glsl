#version 150 core

in vec3 position; // Position in model space

out vec2 texCoord;

// xy component of transform stores quad position 
// zw component of quad stores scale
uniform vec4 transform;

void main(void){
	
	// Model position defined from [-0.5, 0.5] so map it to the range [0, 1]
	texCoord = position.xy + vec2(0.5);      
	
	// Scale quad and position on screen
	vec2 screenPosition = vec2(position) * transform.zw + transform.xy;
	
	// Convert to screen space range [0, 1] (y-axis flips)
	screenPosition.x = screenPosition.x * 2.0 - 1.0;
	screenPosition.y = screenPosition.y * -2.0 + 1.0;
	gl_Position = vec4(screenPosition, 0.0, 1.0);

}