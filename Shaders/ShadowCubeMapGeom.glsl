#version 330

// For each triangle in a mesh, we ouput one triangle per cube face
// For each face we transform the triangle into the light space
// with a different transformation for each of the 6 directions
layout (triangles) in;  
layout (triangle_strip, max_vertices=18) out;

const int NUM_CUBE_FACES = 6;
uniform mat4 lightViewMatrices[NUM_CUBE_FACES];

// Position in light space (for a particular direction)
out vec4 fragWorldPos; 

void main()
{

    for(int face = 0; face < NUM_CUBE_FACES; ++face)
    {
		// Built in glsl variable that redirects the primitive output 
		// to a different cube map face (of the cubemap currently attached
		// to the active framebuffer)
        gl_Layer = face; 
        for(int i = 0; i < gl_in.length(); ++i) 
        {
            fragWorldPos = gl_in[i].gl_Position;
			// Position in light space
            gl_Position = lightViewMatrices[face] * fragWorldPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
}