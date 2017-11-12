#version 150 core

// At each input point create a triangle strip quad rotated to face the camera
layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out; // Four corners of the quad

uniform mat4 viewProjMatrix;
uniform vec3 cameraPos;

out vec2 texCoord;

void main() {
	vec3 pos = gl_in[0].gl_Position.xyz; // Position in world space

	vec3 posToCamera = normalize(cameraPos - pos); // Points from particle position to camera
	vec3 up = vec3(0.0, 1.0, 0.0);				   //TODO: Make a uniform
	vec3 right = cross(posToCamera, up);

	//vec3 bottomLeft = pos - 0.5 * right;
	vec3 bottomLeft = pos - 20.5 * right;
	gl_Position = viewProjMatrix * vec4(bottomLeft, 1.0);
	texCoord = vec2(0.0, 0.0);
	EmitVertex();

	//vec3 topLeft = bottomLeft + vec3(0.0, 1.0, 0.0);
	vec3 topLeft = bottomLeft + vec3(0.0, 20.0, 0.0);
	gl_Position = viewProjMatrix * vec4(topLeft, 1.0);
	texCoord = vec2(0.0, 1.0);
	EmitVertex();

	//vec3 bottomRight = pos + 0.5 * right;
	vec3 bottomRight = pos + 20.5 * right;
	gl_Position = viewProjMatrix * vec4(bottomRight, 1.0);
	texCoord = vec2(1.0, 0.0);
	EmitVertex();

	//vec3 topRight = bottomRight + vec3(0.0, 1.0, 0.0);
	vec3 topRight = bottomRight + vec3(0.0, 20.0, 0.0);
	gl_Position = viewProjMatrix * vec4(topRight, 1.0);
	texCoord  = vec2(1.0, 1.0);
	EmitVertex();

	EndPrimitive();
}


