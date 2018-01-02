#include "ShaderAI.h"

ShaderAI::ShaderAI(const GLchar *vertexPath, const GLchar* fragmentPath, const GLchar* geomPath)
{
	//Retrieve the vertex and fragment source code from the filepath
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	std::string geometryCode;
	std::ifstream gShaderFile;
	//ensures ifstream objects can throw exceptions
	vShaderFile.exceptions(std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::badbit);

	try
	{
		//Open files
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		//Close file handlers
		vShaderFile.close();
		fShaderFile.close();
		//Convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
		if (geomPath != nullptr)
		{
			gShaderFile.open(geomPath);
			std::stringstream gShaderStream;
			gShaderStream << gShaderFile.rdbuf();
			gShaderFile.close();
			geometryCode = gShaderStream.str();
		}
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}

	const GLchar* vShaderCode = vertexCode.c_str();
	const GLchar* fShaderCode = fragmentCode.c_str();

	//Compile Shaders
	GLuint vertex, fragment;
	GLint success;
	GLchar infoLog[INFO_LOG_SIZE];

	//Vertex Shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);

	//Print compile errors if any
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, INFO_LOG_SIZE, nullptr, infoLog);
		std::cout << "Vertex shader compilation error: " << infoLog << std::endl;
	}

	//Fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, nullptr);
	glCompileShader(fragment);


	// if geometry shader is given, compile geometry shader
	unsigned int geometry;
	if (geomPath != nullptr)
	{
		const char * gShaderCode = geometryCode.c_str();
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &gShaderCode, NULL);
		glCompileShader(geometry);

		glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(geometry, INFO_LOG_SIZE, nullptr, infoLog);
			std::cout << "Geometry shader compilation error: " << infoLog << std::endl;
		}
	}

	//Print compile errors if any
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment, INFO_LOG_SIZE, nullptr, infoLog);
		std::cout << "Fragment shader compilation error: " << infoLog << std::endl;
	}
	//Link shader program
	this->Program = glCreateProgram();
	glAttachShader(this->Program, vertex);
	glAttachShader(this->Program, fragment);
	if (geomPath != nullptr)
		glAttachShader(this->Program, geometry);
	glLinkProgram(this->Program);

	// Print linking errors if any
	glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(this->Program, INFO_LOG_SIZE, nullptr, infoLog);
		std::cout << "Shader program linking error\n" << infoLog << std::endl;
	}

	//delete the shaders
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	if (geomPath != nullptr)
		glDeleteShader(geometry);

}

//use the current shader
void ShaderAI::Use()
{
	glUseProgram(this->Program);
}



