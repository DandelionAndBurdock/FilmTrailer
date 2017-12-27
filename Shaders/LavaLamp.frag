// Adapted from Author @patriciogv - 2015
// https://thebookofshaders.com/edit.php#11/lava-lamp.frag

uniform vec2 resolution;
uniform float time;

vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289(((x*34.0)+1.0)*x); }

float snoise(vec2 v) {
    const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                        0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                        -0.577350269189626,  // -1.0 + 2.0 * C.x
                        0.024390243902439); // 1.0 / 41.0
    vec2 i  = floor(v + dot(v, C.yy) );
    vec2 x0 = v -   i + dot(i, C.xx);
    vec2 i1;
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    i = mod289(i); // Avoid truncation effects in permutation
    vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
        + i.x + vec3(0.0, i1.x, 1.0 ));

    vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
    m = m*m ;
    m = m*m ;
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
    m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

float SimplexNoise(vec2 samplePos, float min, float max){
    return snoise(samplePos) * (max - min) + min;
}

void main() {
    const float NUM_SPACES = 1.5; // How many times to wrap the position around
    vec2 st = gl_FragCoord.xy/resolution.xy;
    vec3 colour = vec3(0.0);
    vec2 pos = vec2(st*NUM_SPACES);

    float DF = 0.0;

    // Add time varying noise to each position
    vec2 vel = vec2(time*0.1);
    DF += SimplexNoise(pos + vel, 0.05, 0.80);
    DF += snoise(pos+vel);
    
    // Add a random noise at each position
    float a = snoise(pos)*0.1415;
    DF+= a * sin(time * 0.1);

    float redMin = sin(time) * sin(time) + 0.05;
    float redMax = redMin + 0.05;
     float greenMin = sin(time * 0.5 + 0.5) * sin(time * 0.5 + 0.5) * 0.95;
    float greenMax = greenMin + 0.05;
    float blueMin = sin(time * 0.1 + 2.0) * 0.95 + 0.3;
    float blueMax = blueMin + 0.05;
    colour.x = smoothstep(redMin,redMax,fract(DF));
    colour.y = smoothstep(greenMin,greenMax,fract(DF * 2.0));
    colour.b = smoothstep(blueMin,blueMax,fract(DF));
    gl_FragColor = vec4(colour,1.0);
}