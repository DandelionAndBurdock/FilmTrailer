//TODO: https://learnopengl.com/code_viewer_gh.php?code=src/5.advanced_lighting/3.2.2.point_shadows_soft/3.2.2.point_shadows.fs
#version 330 core
// OmniDirectional Shadows for a single light

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;

uniform samplerCube depthMapTex;

#define MAX_POINT_LIGHTS 1
struct PointLight {
    vec4 position;
	vec4 colour;
	float radius;
};
uniform PointLight pointLights[MAX_POINT_LIGHTS];

uniform vec3 cameraPos;

uniform float ambientStrength;

 in Vertex {
	vec2 texCoord;
	vec3 worldPos;
	vec3 normalWorld;
	vec3 tangentWorld;
	vec3 binormalWorld;
 } IN;

 uniform float farPlane;
 
out vec4 gl_FragColor;


// Returns 1.0 if the scene is in shaodw and 0.0 when it is not
float CalculateShadow(vec3 worldPos);                  

// World space calculation of fragment colour contribution from a point light
vec3 PointLightContribution(PointLight light, vec3 normal, vec3 fragPos, vec3 fragToCamera, float shadow);

void main(){
	// Test if this fragment is in shadow
	float shadow = CalculateShadow(IN.worldPos);
	// Profile if (shadow) {discard;}

	// Sample normal from bump map 
	mat3 TBN = mat3(IN.tangentWorld, IN.binormalWorld, IN.normalWorld);
	vec3 normal = normalize(TBN * (texture(bumpTex, IN.texCoord).rgb * 2.0 - 1.0)); // Change from texture space of [0.0, 1.0] to direction space [-1.0, 1.0]
	
	// Unit vector pointing from fragment position to camera/eye
	vec3 fragToCamera = normalize(cameraPos - IN.worldPos);
	

	vec3 fragColour = PointLightContribution(pointLights[0], normal, IN.worldPos, fragToCamera, shadow);

	
	//gl_FragColor = vec4(IN.normalWorld, 1.0);
	gl_FragColor = vec4(fragColour, 1.0);
}

float CalculateShadow(vec3 worldPos){
	// Vector from fragment position to light
	vec3 fragToLight = worldPos - pointLights[0].position.xyz;

	//  Depth of closest object in the light direction	
    float closestDepth = texture(depthMapTex, fragToLight).r;
	
	// Renormalise closest depth from range [0, 1] to [0, farPlane]
	closestDepth *= farPlane;
	
	// Depth of this fragment to the light
	float currentDepth = length(fragToLight);
	
	// Bias to prevent shadow acne. Give surfaces which are almost perpendicular to the light a smaller bias
	//float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005); 
	float bias = 0.05;

	if (currentDepth > closestDepth + bias)
		return 1.0;
	else
		return 0.0; 
}


// World space calculation of fragment colour contribution from a point light
vec3 PointLightContribution(PointLight light, vec3 normal, vec3 fragPos, vec3 fragToCamera, float shadow)
{
	// Unit vector from fragment position to light
    vec3 fragToLight = normalize(vec3(light.position) - fragPos);
    // Diffuse contribution : Proportional to cosine between normal and incident light ray (Lambert)
	float diffuse = max(dot(normal, fragToLight), 0.0);
    // Specular Contribution : Proportional to the cosine between the normal vector and 
	// the vector half way between the eye vector and light direction
    vec3 halfDir = normalize(fragToLight + fragToCamera);
    float specular = pow(max(dot(normal, halfDir), 0.0), 50.0); //TODO: Map this
    
	// Attenuate contribution  of this light based on distance of fragment from the camera (Simple linear contribution)
    float lightFragDistance = length(vec3(light.position) - fragPos);
    float attenuation = 1.0 - clamp(lightFragDistance / light.radius, 0.0, 1.0);
	diffuse *= attenuation;
    specular *= attenuation;
	
	// Total:
	vec3 diffuseLight = vec3(light.colour) * diffuse * vec3(texture(diffuseTex, IN.texCoord));
	vec3 ambientLight = vec3(light.colour) * ambientStrength * vec3(texture(diffuseTex, IN.texCoord));
    vec3 specularLight = vec3(light.colour) * specular * 0.33;//vec3(texture(material.specular, IN.texCoord));
    vec3 diffuseSpecular = (1.0 - shadow) * (diffuseLight + specularLight);
    return (ambientLight + diffuseSpecular);
}