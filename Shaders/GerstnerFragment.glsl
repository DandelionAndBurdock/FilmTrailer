#version 150 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;


uniform vec3  cameraPos;

#define MAX_POINT_LIGHTS 1
struct PointLight {
    vec4 position;
	vec4 colour;
	float radius;
};
uniform PointLight pointLights[MAX_POINT_LIGHTS];

in Vertex {
	vec3 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent; 
	vec3 binormal; 
	vec3 worldPos;
	vec3 modelPos;
} IN;


out vec4 gl_FragColor;



void main(void) {
vec3 lightPos = pointLights[0].position.xyz;
float lightRadius = pointLights[0].radius;
vec3 lightColour = pointLights[0].colour.xyz;


	vec3 incident = normalize(lightPos - IN.worldPos);
	mat3 TBN = mat3(IN.tangent, IN.binormal, IN.normal);
	vec3 normal = normalize(TBN * (texture (bumpTex, IN.texCoord).rgb * 2.0 - 1.0));

	float lambert = max(0.0, dot(incident, normal));
	float dist = length(lightPos - IN.worldPos);
	float atten = 1.0 - clamp(dist / lightRadius, 0.0, 1.0);

	vec3 viewDir = normalize(cameraPos - IN.worldPos);
	vec3 halfDir = normalize(incident + viewDir);

	float rFactor = max(0.0, dot(halfDir, normal));
	float sFactor = pow(rFactor, 50.0);

	vec4 diffuse = texture(diffuseTex, IN.texCoord);
	vec3 colour = (diffuse.rgb * lightColour.rgb);
	colour += (lightColour.rgb * sFactor) * 0.33;
	gl_FragColor = vec4(colour * atten * lambert, diffuse.a);
	gl_FragColor.rgb += (diffuse.rgb * lightColour.rgb) * 0.1;
}