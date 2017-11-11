#include "TextRenderer.h"

#include <iostream>

#include "../glm/gtc/matrix_transform.hpp"


#include "ShaderManager.h"
#include "Mesh.h"

TextRenderer::TextRenderer(GLuint screenWidth, GLuint screenHeight, std::string fontPath, GLuint fontSize, glm::vec3 fontColour)
{
	this->fontSize = fontSize;
	this->colour = fontColour;

	LoadFont(fontPath);
	LoadShader(screenWidth, screenHeight);

	quad = Mesh::GenerateNullQuad();
}

TextRenderer::~TextRenderer() {
	delete quad;
}

void TextRenderer::LoadFont(std::string fontPath)
{
	// Clear any previously loaded characters
	characters.clear();

	// In FreeType a face describes a given fontface and style e.g. Times New Roman Italic
	FT_Face face;
	// Handle for the FreeType library instance
	FT_Library ft;

	InitialiseFontLibrary(ft, face, fontPath);
	LoadCharacters(face);
	// No longer need FreeType library
	CleanUp(ft, face);

}

void TextRenderer::CleanUp(FT_Library& ft, FT_Face& face) {
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}


void TextRenderer::RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	SHADER_MANAGER->SetShader("TextShader");
	for (auto c = text.begin(); c != text.end(); ++c)
	{
		Character ch = characters[*c];

		// Position the glyph correctly on the line (depends on character)
		GLfloat xpos = x + ch.bearing.x * scale;
		GLfloat ypos = y - ch.bearing.y * scale;

		// Set width and height of glyph
		GLfloat w = ch.size.x * scale;
		GLfloat h = ch.size.y * scale;

		// Update VBO for each character
		glm::vec3 verts[] =
		{
			{ xpos,     ypos,       0.0, }, // Bottom left
			{ xpos + w, ypos + h,   0.0,},  // Top right
			{ xpos,     ypos + h,   0.0,}, // Top Left
			{ xpos + w, ypos,       0.0,}, // Bottom right
		};

		// Render glyph texture over quad
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ch.textureID);
		quad->BufferVertices(verts);
		quad->Draw();

		// Now advance cursors for next glyph
		// Glyph metrics are stored in units of 1/64th of a pixel so need to multiply by 64 (or bitshift by 6)
		x += (ch.advance >> 6) * scale; 
	}
	glDisable(GL_BLEND);
}

void TextRenderer::RenderTextSplit(std::string text, GLfloat x, GLfloat y, GLfloat scale, GLuint max)
{
	GLuint numberOfLines = text.size() / max;
	const GLuint LINE_SPACING = 18.0f;
	if (numberOfLines == 0)
	{
		RenderText(text, x, y, scale);
	}
	else
	{
		auto pos = text.find(' ', max);
		std::string line = text.substr(0, pos);
		std::string remainder = text.substr(pos + 1, text.size());
		RenderText(line, x, y, scale);
		RenderTextSplit(remainder, x, y + LINE_SPACING, scale, max);
	}
}

void TextRenderer::InitialiseFontLibrary(FT_Library& ft, FT_Face& face, const std::string& fontPath) {
	// Create a new instance of FreeType library
	if (FT_Init_FreeType(&ft)) // Note: Freetype functions return non-zero value to signal error
	{
		std::cout << "ERROR::FREETYPE: Failed to init FreeType Library" << std::endl;
	}

	// Third parameter in FT_New_Face is for multiple font faces embedded in a single file,
	// We don't care about this so set it to zero
	if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

	// Set the pixel size for each character to fontSize x fontSize pixels. (A value of 0 for one dimension parameter means "same as the other one")
	FT_Set_Pixel_Sizes(face, 0, fontSize);
}

void TextRenderer::LoadCharacters(FT_Face& face) {
	// OpenGL has 4-byte default alignment, this ensures we don't have alignment issues with our single byte per pixel they can have any possible width. 
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// For now we restrict ourselves to the first 128 characters of the ASCII table
	//TODO: Define magic numbers
	for (GLubyte c = 0; c < 128; ++c)
	{

		// FT_Load_Char converts character code to a font glyph index and
		// then loads the corresponding glyph image
		// FT_LOAD_RENDER tells FreeType to create an 8-bit grayscale bitmap image 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		
		GLuint textureID = GenerateTextureFromFace(face);
		SetTextureParameters();
		StoreCharacter(c, face, textureID);
	}
}


GLuint TextRenderer::GenerateTextureFromFace(FT_Face& face) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(
		GL_TEXTURE_2D,					// Send to texture bound to GL_TEXTURE_2D
		0,								// Mip-map level
		GL_RED,							// Monocolour output
		face->glyph->bitmap.width,		// Texture width
		face->glyph->bitmap.rows,		// Texture height
		0,								// Legacy parameter (ignore)
		GL_RED,							// Format of source image
		GL_UNSIGNED_BYTE,				// Format of data
		face->glyph->bitmap.buffer		// Image date
	);
	return textureID;
}

void TextRenderer::SetTextureParameters() {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void TextRenderer::StoreCharacter(char c, FT_Face& face, GLuint textureID) {
	Character character = {
		textureID,
		glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
		glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
		face->glyph->advance.x
	};
	characters.insert(std::pair<GLchar, Character>(c, character));
}

void TextRenderer::LoadShader(GLuint screenWidth, GLuint screenHeight) {
	SHADER_MANAGER->SetShader("TextShader");
	SHADER_MANAGER->SetUniform("TextShader", "projMatrix", glm::ortho(0.0f, static_cast<GLfloat>(screenWidth), static_cast<GLfloat>(screenHeight), 0.0f));
	SHADER_MANAGER->SetUniform("TextShader", "text", 0);
	SHADER_MANAGER->SetUniform("TextShader", "textColour", colour);
}