#include "Mesh.h"

Mesh::Mesh(){
	for (int i = 0; i < MAX_BUFFER; ++i){
		bufferObject[i] = 0;
	}

	glGenVertexArrays(1, &arrayObject); 

	indices = nullptr;
	numIndices = 0;
	numVertices = 0;
	vertices = nullptr;
	colours = nullptr;
	type = GL_TRIANGLES;
	textureCoords = nullptr;
	normals = nullptr;
}


Mesh::~Mesh(){
	glDeleteVertexArrays(1, &arrayObject);
	glDeleteBuffers(MAX_BUFFER, bufferObject);

	delete[] indices;
	delete[] vertices;
	delete[] colours;
	delete[] textureCoords;
	delete[] normals;
}

Mesh* Mesh::GenerateTriangle(){
	const int VERTS_PER_TRIANGLE = 3;
	Mesh* m = new Mesh();

	m->numVertices = VERTS_PER_TRIANGLE;

	m->vertices = new Vector3[m->numVertices];
	m->vertices[0] = Vector3(+0.0f, +0.5f, +0.0f);
	m->vertices[1] = Vector3(+0.5f, -0.5f, +0.0f);
	m->vertices[2] = Vector3(-0.5f, -0.5f, +0.0f);

	m->textureCoords = new Vector2[m->numVertices];
	m->textureCoords[0] = Vector2(0.5f, 0.0f);
	m->textureCoords[1] = Vector2(1.0f, 1.0f);
	m->textureCoords[2] = Vector2(0.0f, 1.0f);

	m->colours = new Vector4[m->numVertices];
	m->colours[0] = Vector4(+1.0f, +0.0f, +0.0f, +1.0f);
	m->colours[1] = Vector4(+0.0f, +1.0f, +0.0f, +1.0f);
	m->colours[2] = Vector4(+0.0f, +0.0f, +1.0f, +1.0f);

	m->BufferData();

	return m;
}

Mesh* Mesh::GenerateQuad(){
	const int VERTS_PER_QUAD = 4; 
	Mesh* m = new Mesh();

	m->type = GL_TRIANGLE_STRIP;
	m->numVertices = VERTS_PER_QUAD;

	m->vertices = new Vector3[m->numVertices];
	m->vertices[0] = Vector3(-1.0f, -1.0f, +0.0f);
	m->vertices[1] = Vector3(-1.0f, +1.0f, +0.0f);
	m->vertices[2] = Vector3(+1.0f, -1.0f, +0.0f);
	m->vertices[3] = Vector3(+1.0f, +1.0f, +0.0f);

	m->colours = new Vector4[m->numVertices];
	m->colours[0] = Vector4(+1.0f, +0.0f, +0.0f, +1.0f);
	m->colours[1] = Vector4(+0.0f, +1.0f, +0.0f, +1.0f);
	m->colours[2] = Vector4(+0.0f, +0.0f, +1.0f, +1.0f);
	m->colours[3] = Vector4(+0.0f, +1.0f, +0.0f, +1.0f);

	m->textureCoords = new Vector2[m->numVertices];
	m->textureCoords[0] = Vector2(0.0f, 1.0f);
	m->textureCoords[1] = Vector2(0.0f, 0.0f);
	m->textureCoords[2] = Vector2(1.0f, 1.0f);
	m->textureCoords[3] = Vector2(1.0f, 0.0f);

	m->BufferData();

	return m;
}



void Mesh::BufferData() {
	const int FLOATS_PER_VERTEX = 3;
	const int FLOATS_PER_COLOUR = 4;
	const int STRIDE = 0;
	const int OFFSET = 0;

	glBindVertexArray(arrayObject);

	glGenBuffers(1, &bufferObject[VERTEX_BUFFER]);
	glBindBuffer(GL_ARRAY_BUFFER, bufferObject[VERTEX_BUFFER]);
	glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector3), 
				 vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(VERTEX_BUFFER, FLOATS_PER_VERTEX, GL_FLOAT,
							GL_FALSE, STRIDE, OFFSET);
	glEnableVertexAttribArray(VERTEX_BUFFER);

	if (textureCoords) {
		glGenBuffers(1, &bufferObject[TEXTURE_BUFFER]);
		glBindBuffer(GL_ARRAY_BUFFER, bufferObject[TEXTURE_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector2),
			textureCoords, GL_STATIC_DRAW);
		glVertexAttribPointer(TEXTURE_BUFFER, 2, GL_FLOAT, GL_FALSE, STRIDE, OFFSET);
		glEnableVertexAttribArray(TEXTURE_BUFFER);
	}

	if (colours){
		glGenBuffers(1, &bufferObject[COLOUR_BUFFER]);
		glBindBuffer(GL_ARRAY_BUFFER, bufferObject[COLOUR_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector4),
					colours, GL_STATIC_DRAW);
		glVertexAttribPointer(COLOUR_BUFFER, FLOATS_PER_COLOUR, GL_FLOAT, 
							  GL_FALSE, STRIDE, OFFSET);
		glEnableVertexAttribArray(COLOUR_BUFFER);

	}
	if (indices) {
		glGenBuffers(1, &bufferObject[INDEX_BUFFER]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferObject[INDEX_BUFFER]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLuint), indices, GL_STATIC_DRAW);
	}
	if (normals) {
		glGenBuffers(1, &bufferObject[NORMAL_BUFFER]);
		glBindBuffer(GL_ARRAY_BUFFER, bufferObject[NORMAL_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector3),
			normals, GL_STATIC_DRAW);
		glVertexAttribPointer(NORMAL_BUFFER, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(NORMAL_BUFFER);
	}

	glBindVertexArray(0);
}

void Mesh::Draw() {

	glBindVertexArray(arrayObject);
	if (bufferObject[INDEX_BUFFER]) {
		glDrawElements(type, numIndices, GL_UNSIGNED_INT, 0);
	}
	else {
		glDrawArrays(type, 0, numVertices);
	}

	glBindVertexArray(0);
}

void Mesh::GenerateNormals() {
	if (!normals) {
		normals = new Vector3[numVertices];
	}

	for (GLuint i = 0; i < numVertices; ++i) {
		normals[i] = Vector3();
	}

	if (indices) {
		for (GLuint i = 0; i < numIndices; i += 3) {
			unsigned int a = indices[i];
			unsigned int b = indices[i + 1];
			unsigned int c = indices[i + 2];

			Vector3 normal = Vector3::Cross(vertices[b] - vertices[a],
				vertices[c] - vertices[a]);

			normals[a] += normal;
			normals[b] += normal;
			normals[c] += normal;
		}
	}
	else {
		for (GLuint i = 0; i < numVertices; i+=3) {
			Vector3& a = vertices[i];
			Vector3& b = vertices[i + 1];
			Vector3& c = vertices[i + 2];

			Vector3 normal = Vector3::Cross(b - a, c - a);

			normals[i] = normal;
			normals[i + 1] = normal;
			normals[i + 2] = normal;
		}
	}

	for (GLuint i = 0; i < numVertices; ++i) {
		normals[i].Normalise();
	}
}