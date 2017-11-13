 #version 150 core
 
 uniform mat4 modelMatrix;
 uniform mat4 viewMatrix;
 uniform mat4 projMatrix;
 
 in vec3 position;
 in vec2 texCoord;
 in vec3 normal;
 
 out Vertex {
 vec2 texCoord;
 vec3 worldPos;
 vec3 normalWorld;
 } OUT;
 
 void main() {
 OUT.texCoord = texCoord;
 OUT.normalWorld = mat3(transpose(inverse(model))) * normal;
 OUT.worldPos = (modelMatrix * vec4(position, 1)).xyz;
  gl_Position = (projMatrix * viewMatrix * modelMatrix) * vec4(position, 1.0);
 }