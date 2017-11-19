#version 150 core

#define MAX_DIR_LIGHTS 1

uniform sampler2D reflectionTex;
uniform sampler2D bumpTex;
uniform sampler2D refractionTex;
uniform sampler2D dudvMapTex;
uniform sampler2D depthMapTex;

uniform float time; // Time in sec

uniform vec3 cameraPos; // Position of the camera in world space

uniform float nearPlane;
uniform float farPlane;

struct DirectionalLight {
	vec4 direction;
	vec4 colour;
};
uniform DirectionalLight directionalLights[MAX_DIR_LIGHTS];

in vec4 clipSpacePos;
in vec2 texCoord;
in vec3 fragWorldPos;

out vec4 gl_FragColor;

const float WAVE_STRENGTH = 0.02;
const float WAVE_SPEED = 0.03;

// Alpha value of water will go from 0 to 1 between
// a depth of 0 and MAX_EDGE_DEPTH. Gives softer edges
const float MAX_EDGE_DEPTH = 30.0f;
// Water with a depth higher than deep water will have a slightly different colour
const float DEEP_WATER = 10000.0f;

const vec4 BLUE_GREEN_COLOUR = vec4(0.0, 0.4, 0.6, 1.0);
const vec4 DEEP_WATER_COLOUR = vec4(0.0, 0.47, 0.75, 1.0);
// Uncomment if don't want deep water effect
const float REFLECTIVE_POWER = 0.5; // Strength of reflection in the Fresnel effect
void main(){

// Position in normalised device coordinates
vec2 ndcPos = (clipSpacePos.xy / clipSpacePos.w);

// Move to (u, v) space
vec2 uvPos = (ndcPos / 2.0) + 0.5;

vec2 refractSampleCoords = uvPos;
// Sample reflection image upside down
vec2 reflectSampleCoords = vec2(uvPos.x, -uvPos.y);

// Depth of the water from camera viewpoint (in non-linear depth space)
float floorCameraDistance = texture(depthMapTex, refractSampleCoords).r;
// Perform linearisation of depth value
// (1. Reverse offset from NDC to clip space, 2. Reverse perspective divide, 3. Reverse projMatrix multiplication)
floorCameraDistance = 2.0 * nearPlane * farPlane / (farPlane + nearPlane - (2.0 * floorCameraDistance - 1.0) * (farPlane - nearPlane));

// Distance from the camera to water surface
float surfaceCameraDistance = gl_FragCoord.z;
// Linearise
surfaceCameraDistance = 2.0 * nearPlane * farPlane / (farPlane + nearPlane - (2.0 * surfaceCameraDistance - 1.0) * (farPlane - nearPlane));

// Real depth of the water in world space
float waterDepth = floorCameraDistance - surfaceCameraDistance;



// Sample at varying points with time so that surface is not perfect mirror
float distortDistance = time * WAVE_SPEED;
// Combinining multiple distortions in diffent directions gives a more realistic feel for the water
vec2 distortTexCoords = 0.1 * texture(dudvMapTex, vec2(texCoord.x + distortDistance, texCoord.y)).rg;
distortTexCoords = texCoord + vec2(distortTexCoords.x, distortTexCoords.y + distortDistance);
vec2 totalDistortion = WAVE_STRENGTH * (texture(dudvMapTex, distortTexCoords).rg * 2.0 - 1.0); // Convert dudv colours to range[-1, 1]

refractSampleCoords += totalDistortion;
reflectSampleCoords += totalDistortion;

// Ugly bit of code to stop openGL tiling wrap around giving strange distortion at very edge of screen
refractSampleCoords = clamp(refractSampleCoords, 0.01, 0.99);
reflectSampleCoords.x = clamp(reflectSampleCoords.x, 0.01, 0.99);
reflectSampleCoords.y = clamp(reflectSampleCoords.y, -0.99, 0.01);

vec4 refractColour = texture(refractionTex, refractSampleCoords);
refractColour = mix(refractColour, DEEP_WATER_COLOUR, clamp(waterDepth / DEEP_WATER, 0.0, 1.0));
vec4 reflectColour = texture(reflectionTex, reflectSampleCoords);

// Calculate specular hightlights : (No diffuse  component for simple water quads)
DirectionalLight light = directionalLights[0];

vec3 normal = texture(bumpTex, distortTexCoords).rgb;
// Assume quad is flat so can avoid TBN matrix
normal = normalize(vec3(normal.r * 2.0 - 1.0, normal.b, normal.g * 2.0 - 1.0));

// Unit vector from fragment position to light
vec3 fragToLight = vec3(normalize(-light.direction));

// Unit vector from fragment to camera in worldSpace
vec3 fragToCamera = normalize(cameraPos - fragWorldPos);

// Specular Contribution : Proportional to the cosine between the normal vector and 
// the vector half way between the eye vector and light direction
vec3 halfDir = normalize(fragToLight + fragToCamera);
float specular = pow(max(dot(normal, halfDir), 0.0), 50.0); //If time: Map this //vec3(texture(material.specular, IN.texCoord));
vec3 specularLight = vec3(light.colour) * specular * 0.33;

// Calculate Fresnel component -> Water more transparent from deep angles and more reflective from shallow angles
float refractWeighting = dot(fragToCamera, vec3(0, 1, 0));

vec4 fragColor = mix(reflectColour, refractColour, refractWeighting);
// Give blue-green tint
gl_FragColor = mix(fragColor, BLUE_GREEN_COLOUR, 0.2) + vec4(specularLight, 0.0);
gl_FragColor.a = clamp(waterDepth/ MAX_EDGE_DEPTH, 0.0, 1.0);
}
