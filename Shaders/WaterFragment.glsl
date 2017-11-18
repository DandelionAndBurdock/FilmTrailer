#version 150 core

uniform sampler2D reflectionTex;
uniform sampler2D refractionTex;
uniform sampler2D dudvMapTex;

uniform float time; // Time in sec

in vec4 clipSpacePos;
in vec2 texCoord;

out vec4 gl_FragColor;

const float WAVE_STENGTH = 0.04;
const float WAVE_SPEED = 0.03;

const vec4 BLUE_GREEN_COLOUR = vec4(0.0, 0.3, 0.5, 1.0);

void main(){

// Position in normalised device coordinates
vec2 ndcPos = (clipSpacePos.xy / clipSpacePos.w);

// Move to (u, v) space
vec2 uvPos = (ndcPos / 2.0) + 0.5;

vec2 refractSampleCoords = uvPos;
// Sample reflection image upside down
vec2 reflectSampleCoords = vec2(uvPos.x, -uvPos.y);

// Convert dudv colours to range[-1, 1]
// Sample at varying points
float distortDistance = time * WAVE_SPEED;
vec2 distortion = WAVE_STENGTH * (texture(dudvMapTex, vec2(texCoord.x + sin(distortDistance), texCoord.y + cos(distortDistance))).rg * 2.0 - 1.0);

refractSampleCoords += distortion;
reflectSampleCoords += distortion;

// Ugly bit of code to stop openGL tiling wrap around giving strange distortion at very edge of screen
refractSampleCoords = clamp(refractSampleCoords, 0.01, 0.99);
reflectSampleCoords.x = clamp(reflectSampleCoords.x, 0.01, 0.99);
reflectSampleCoords.y = clamp(reflectSampleCoords.y, -0.99, 0.01);

vec4 refractColour = texture(refractionTex, refractSampleCoords);
vec4 reflectColour = texture(reflectionTex, reflectSampleCoords);


vec4 fragColor = mix(reflectColour, refractColour, 0.5);
// Give blue-green tint
gl_FragColor = mix(fragColor, BLUE_GREEN_COLOUR, 0.2);
}
