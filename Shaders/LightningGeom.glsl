#version 330 core

// At each input point create a triangle strip quad rotated to face the camera
layout(lines) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out; // Four corners of the quad

uniform mat4 viewProjMatrix;
uniform vec3 cameraPos;

out vec2 texCoord;

in float width[];

void main() {
	vec3 up = vec3(0.0, 1.0, 0.0);				   //TODO: Make a uniform

	vec3 pos = gl_in[0].gl_Position.xyz; // Position in world space
	float thickness = width[0];
	
	vec3 posToCamera = normalize(cameraPos - pos); // Points from particle position to camera
	vec3 right = cross(posToCamera, up);
	
	
	// Designate [0] to be the top left point
	gl_Position = viewProjMatrix * vec4(pos, 1.0);
	texCoord = vec2(0.0, 1.0);
	EmitVertex();
	
	// Top right point
	gl_Position = viewProjMatrix * vec4(pos + thickness * right, 1.0);
	texCoord = vec2(1.0, 1.0);
	EmitVertex();
	
	// Designate [1] to be bottom left point
	pos = gl_in[1].gl_Position.xyz;
	gl_Position = viewProjMatrix * vec4(pos, 1.0);
	texCoord = vec2(0.0, 0.0);
	EmitVertex();
	
	// Bottom Right
	gl_Position = viewProjMatrix * vec4(pos + thickness * right, 1.0);
	texCoord = vec2(1.0, 0.0);
	EmitVertex();

	EndPrimitive();
}


