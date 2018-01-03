 #version 150 core
 
 uniform mat4 modelMatrix;
 uniform mat4 viewMatrix;
 uniform mat4 projMatrix;
 
 uniform sampler2D heightMapTex;
 
 // Normal x, y, z and distance from the origin
 // Since we are only dealing with horizontal 
 // planes the normal will be either (0, 1, 0)
 // or (0, -1, 0) and d is the height above the origin 
 uniform vec4 clipPlane;
 
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
 float height = 100.0 * texture(heightMapTex, texCoord).r;
vec3 newPosition = vec3(position.x, position.y + height, position.z);
 vec4 worldPos = modelMatrix * vec4(newPosition, 1.0);

 // Distance of the vertex from the clip plane
  gl_ClipDistance[0] = dot(worldPos, clipPlane);
  

 
 
 OUT.worldPos = worldPos.xyz;

 
 OUT.texCoord = texCoord;
 
 // Transforms normals from model space to world space
 mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
 OUT.normalWorld = normalize(normalMatrix * normalize(normal));
 OUT.tangentWorld = normalize(normalMatrix * normalize(tangent));
 OUT.binormalWorld = normalize(normalMatrix * normalize(cross(normal, tangent)));
 
 
 
  gl_Position = (projMatrix * viewMatrix * modelMatrix) * vec4(newPosition, 1.0);
 }