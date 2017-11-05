// Adapted from wikibooks: https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_01
// Additional Comments from FreeType documentation: https://www.freetype.org/freetype2/docs/tutorial/step1.html
#pragma once

#include <map>

#include <GL\glew.h>

#include "../glm/vec2.hpp"
#include "Shader.h"

#include "../FreeType/include/ft2build.h"
#include FT_FREETYPE_H // This is a macro for installation instructions defined within ft2build.h

// Holds state information for a FreeType character
struct Character
{
	GLuint textureID;	// glyph texture ID
	glm::ivec2 size;    // width/ height of glyph quad
	glm::ivec2 bearing; // vector to top left corner of glyph quad
	GLuint advance;     // Pixel distance from origin to next glyph
};


// A text renderer class loads and displays a font using FreeType library.
class TextRenderer
{
public: 
	// Constructor
	TextRenderer(GLuint screenWidth, GLuint screenHeight, std::string fontPath, GLuint fontSize, glm::vec3 fontColour);
	~TextRenderer();



	// Renders a string of text at position x and y
	void RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale);
	// Render max characters of text per line. Splits text over several lines if too long
	void RenderTextSplit(std::string text, GLfloat x, GLfloat y, GLfloat scale, GLuint max = 60);

private:
	// Pre-compile characters from the given font
	void LoadFont(std::string fontPath);

	// Load shader and configure uniforms
	void LoadShader(GLuint screenWidth, GLuint screenHeight);

	// Load FreeType library
	void InitialiseFontLibrary(FT_Library& ft, FT_Face& face, const std::string& fontPath);
	// Pre-load first 128 ASCII characters and store them 
	void LoadCharacters(FT_Face& face);
	// Destroy FreeType 
	void CleanUp(FT_Library& ft, FT_Face& face);

	// Generate texture of alphabet using the grayscale bitmap image loaded into face->glyph->bitmap.
	GLuint GenerateTextureFromFace(FT_Face& face);
	// Set currently bound texture parameters for text
	void SetTextureParameters();
	// Create a character from face texture and store in character map with key 'c'
	void StoreCharacter(char c, FT_Face& face, GLuint textureID);



	// Vertex Array Object
	GLuint VAO, VBO;

	// Holds list of pre-compiled characters
	std::map<GLchar, Character> characters;

	// Text rendering shader
	Shader* shader;

	// Text colour
	glm::vec3 colour;

	// Font size in pixels
	GLuint fontSize;

	// Each character will be stored on a texture quad which can be drawn to screen
	Mesh* quad;
};




