//TODO: https://learnopengl.com/code_viewer_gh.php?code=src/2.lighting/6.multiple_lights/6.multiple_lights.fs
#version 150 core

#define MAX_POINT_LIGHTS 4

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;

struct PointLight {
    vec3 position;
	vec3 colour;
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

out vec4 gl_FragColor;

// World space calculation of fragment colour contribution from a point light
vec3 PointLightContribution(PointLight light, vec3 normal, vec3 fragPos, vec3 fragToCamera)
{
	// Unit vector from fragment position to light
    vec3 fragToLight = normalize(light.position - fragPos);
    // Diffuse contribution : Proportional to cosine between normal and incident light ray (Lambert)
	float diffuse = max(dot(normal, fragToLight), 0.0);
    // Specular Contribution : Proportional to the cosine between the normal vector and 
	// the vector half way between the eye vector and light direction
    vec3 halfDir = normalize(fragToLight + fragToCamera);
    float specular = pow(max(dot(normal, halfDir), 0.0), 50.0); //TODO: Map this
    
	// Attenuate contribution  of this light based on distance of fragment from the camera (Simple linear contribution)
    float lightFragDistance = length(light.position - fragPos);
    float attenuation = 1.0 - clamp(lightFragDistance / light.radius, 0.0, 1.0);
	diffuse *= attenuation;
    specular *= attenuation;
	
	// Total:
	vec3 diffuseLight = light.colour * diffuse * vec3(texture(diffuseTex, IN.texCoord));
	vec3 ambientLight = (ambientStrength / MAX_POINT_LIGHTS) * vec3(texture(diffuseTex, IN.texCoord));
    vec3 specularLight = light.colour * specular * 0.33;//vec3(texture(material.specular, IN.texCoord));
    return (ambientLight + diffuseLight + specularLight);
}

void main(){
	// Normalise normal in case interpolation has messed it up
	vec3 norm = normalize(IN.normalWorld);
	vec3 binormal = normalize(IN.binormalWorld);
	vec3 tangent = normalize(IN.tangentWorld);
	
	// Sample normal from bump map 
	//mat3 TBN = mat3(IN.tangentWorld, IN.binormalWorld, IN.normalWorld);
	mat3 TBN = mat3(tangent, binormal, norm);
	vec3 normal = normalize(TBN * (texture(bumpTex, IN.texCoord).rgb * 2.0 - 1.0)); // Change from texture space of [0.0, 1.0] to direction space [-1.0, 1.0]
	
	// Unit vector pointing from fragment position to camera/eye
	vec3 fragToCamera = normalize(cameraPos - IN.worldPos);
	
	vec3 fragColour = vec3(0.0);
	// Point Lights
	for (int i = 0; i < MAX_POINT_LIGHTS; i++){
		fragColour += PointLightContribution(pointLights[i], normal, IN.worldPos, fragToCamera);
	}
	
	gl_FragColor = vec4(fragColour, 1.0);
}