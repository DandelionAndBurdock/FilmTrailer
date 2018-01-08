#version 150 core

out vec4 gl_FragColor;

uniform vec3 coneColour;
void main(){
gl_FragColor = vec4(coneColour, 0.5);
}
