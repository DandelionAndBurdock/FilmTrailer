 #version 150 core
 
 uniform mat4 modelMatrix;
 uniform mat4 viewMatrix;
 uniform mat4 projMatrix;
 
in vec3 position;
in vec2 texCoord;
in vec3 normal;
in vec3 tangent;
 
 out Vertex {
	vec2 texCoord;
	vec3 worldPos;
	vec3 normalWorld;
	vec3 tangentWorld;
	vec3 binormalWorld;
 } OUT;
 
 void main() {
 OUT.texCoord = texCoord;
 
 // Transforms normals from model space to world space
 mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
 OUT.normalWorld = normalize(normalMatrix * normal);
 OUT.tangentWorld = normalize(normalMatrix * normalize(tangent));
 OUT.binormalWorld = normalize(normalMatrix * normalize(cross(normal, tangent)));
 OUT.worldPos = (modelMatrix * vec4(position, 1)).xyz;
  gl_Position = (projMatrix * viewMatrix * modelMatrix) * vec4(position, 1.0);
 }