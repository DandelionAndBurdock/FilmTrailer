#version 150
uniform sampler2D diffuseTex;

uniform float time;

in Vertex {
	vec2 texCoord;
} IN;

//in Vertex	{
//	vec4 	colour;
//	vec2 	texCoord;
//	vec3 	normal;
//	vec3 	tangent;
//	vec3 	worldPos;
//} IN;


vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289(((x*34.0) + 1.0)*x); }

float snoise(vec2 v) {
	const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
		0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
		-0.577350269189626,  // -1.0 + 2.0 * C.x
		0.024390243902439); // 1.0 / 41.0
	vec2 i = floor(v + dot(v, C.yy));
	vec2 x0 = v - i + dot(i, C.xx);
	vec2 i1;
	i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
	vec4 x12 = x0.xyxy + C.xxzz;
	x12.xy -= i1;
	i = mod289(i); // Avoid truncation effects in permutation
	vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0))
		+ i.x + vec3(0.0, i1.x, 1.0));

	vec3 m = max(0.5 - vec3(dot(x0, x0), dot(x12.xy, x12.xy), dot(x12.zw, x12.zw)), 0.0);
	m = m*m;
	m = m*m;
	vec3 x = 2.0 * fract(p * C.www) - 1.0;
	vec3 h = abs(x) - 0.5;
	vec3 ox = floor(x + 0.5);
	vec3 a0 = x - ox;
	m *= 1.79284291400159 - 0.85373472095314 * (a0*a0 + h*h);
	vec3 g;
	g.x = a0.x  * x0.x + h.x  * x0.y;
	g.yz = a0.yz * x12.xz + h.yz * x12.yw;
	return 130.0 * dot(m, g);
}

float SimplexNoise(vec2 samplePos, float min, float max) {
	return snoise(samplePos) * (max - min) + min;
}

// Random Function relies on rapid modulation of high frequency sine wave
//http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
highp float RandZeroToOne(vec2 seed)
{
	highp float a = 12.9898;
	highp float b = 78.233;
	highp float c = 43758.5453;
	highp float dt = dot(seed.xy, vec2(a, b));
	highp float sn = mod(dt, 3.14);
	return fract(sin(sn) * c);
}

out vec4 gl_FragColor;

void main(void){
	// 1. Sample Image
	gl_FragColor = texture(diffuseTex, IN.texCoord);

	// 2. Add static noise
	float noise = SimplexNoise(1000.0f * IN.texCoord * (sin(time / 1000.0f) + vec2(345.0f, 201.0f)), 0.0, 0.1);
	gl_FragColor += vec4(noise, noise, noise, 0.0);


	float cutOff = RandZeroToOne(50.0f * vec2(sin(time), cos(time)));
	if (cutOff < 0.2) {
		// Pick a random centre point to show scratches
		float radius = 0.1f;
		float x = RandZeroToOne(500.0f * vec2(sin(time / 100.0f), cos(time / 100.0f)));
		float y = RandZeroToOne(500.0f * vec2(cos(time / 100.0f), sin(time / 100.0f)));
		float d = distance(IN.texCoord, vec2(x, y));
		if (d < radius) {
			// Generate the scratch
			float pi = 3.141592;
			float turbulence = SimplexNoise(IN.texCoord, 0.0, 2.5);
			float colour = 0.5 + (sin((IN.texCoord.x * 8.0 + IN.texCoord.y * 4.0 + turbulence) * pi + time / 1000.0f) * 0.5);
			colour = clamp(colour * 1000.0 + 0.35, 0.0, 1.0);
			gl_FragColor.xyz *= colour;
		}

	}

}
//
//// Pick a random spot to show scratches
//float dist = 1.0 / ScratchValue;
//float d = distance(vUv, vec2(RandomValue * dist, RandomValue * dist));
//if (d < 0.4)
//{
//	// Generate the scratch
//	float xPeriod = 8.0;
//	float yPeriod = 1.0;
//	float pi = 3.141592;
//	float phase = TimeLapse;
//	float turbulence = snoise(vUv * 2.5);
//	float vScratch = 0.5 + (sin(((vUv.x * xPeriod + vUv.y * yPeriod + turbulence)) * pi + phase) * 0.5);
//	vScratch = clamp((vScratch * 10000.0) + 0.35, 0.0, 1.0);
//
//	finalColour.xyz *= vScratch;
//}