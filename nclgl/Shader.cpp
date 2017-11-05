#include "Shader.h"

#include <iostream>
Shader::Shader(std::string vFile, std::string fFile, std::string gFile)	{
	program = glCreateProgram();
	objects[SHADER_VERTEX] = GenerateShader(vFile, GL_VERTEX_SHADER);
	objects[SHADER_FRAGMENT] = GenerateShader(fFile, GL_FRAGMENT_SHADER);
	objects[SHADER_GEOMETRY] = 0;

	if (!gFile.empty()) {
		objects[SHADER_GEOMETRY] = GenerateShader(gFile, GL_GEOMETRY_SHADER);
		glAttachShader(program, objects[SHADER_GEOMETRY]);
	}

	glAttachShader(program, objects[SHADER_VERTEX]);
	glAttachShader(program, objects[SHADER_FRAGMENT]);

	SetDefaultAttributes();
}

Shader::~Shader(void)	{
	for (int i = 0; i < 3; ++i) {
		glDetachShader(program, objects[i]);
		glDeleteShader(objects[i]);
	}
	glDeleteProgram(program);
}

bool	Shader::LoadShaderFile(std::string from, std::string &into)	{
	std::ifstream	file;
	std::string		temp;

	std::cout << "Loading shader text from " << from << std::endl << std::endl;

	file.open(from.c_str());
	if (!file.is_open()){
		std::cout << "File does not exist!" << std::endl;
		return false;
	}

	while (!file.eof()){
		getline(file, temp);
		into += temp + "\n";
	}

	std::cout << into << std::endl << std::endl;

	file.close();
	std::cout << "Loaded shader text!" << std::endl << std::endl;
	return true;
}

GLuint	Shader::GenerateShader(std::string from, GLenum type)	{
	std::cout << "Compiling Shader..." << std::endl;

	std::string load;
	if (!LoadShaderFile(from, load)) {
		std::cout << "Compiling failed!" << std::endl;
		loadFailed = true;
		return 0;
	}
	shaderText.push_back(load);

	GLuint shader = glCreateShader(type);

	const char *chars = load.c_str();
	glShaderSource(shader, 1, &chars, NULL);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE)	{
		std::cout << "Compiling failed!" << std::endl;
		char error[512];
		glGetInfoLogARB(shader, sizeof(error), NULL, error);
		std::cout << error;
		loadFailed = true;
		return 0;
	}
	std::cout << "Compiling success!" << std::endl << std::endl;
	loadFailed = false;

	return shader;
}

bool Shader::LinkProgram()	{
	if (loadFailed) {
		return false;
	}
	glLinkProgram(program);

	GLint code;
	glGetProgramiv(program, GL_LINK_STATUS, &code);
	if (!code){
		char error[512];
		glGetProgramInfoLog(program, sizeof(error), NULL, error);
		std::cout << "Linking failed\n" << error << std::endl;
	}
	else {
		SetUniformLocations();
		std::cout << "Linking success" << std::endl;
	}

	return code == GL_TRUE ? true : false;
}

void	Shader::SetDefaultAttributes()	{
	glBindAttribLocation(program, VERTEX_BUFFER, "position");
	glBindAttribLocation(program, COLOUR_BUFFER, "colour");
	glBindAttribLocation(program, NORMAL_BUFFER,  "normal");
	//glBindAttribLocation(program, TANGENT_BUFFER, "tangent");
	glBindAttribLocation(program, TEXTURE_BUFFER, "texCoord");
}

//TODO: Refactor
void Shader::SetUniformLocations() {
	const std::string keyword = std::string("uniform");
	glUseProgram(program);
	for (const auto& shader : shaderText) {
		std::size_t uniformPosition = shader.find(keyword);
		while (uniformPosition != std::string::npos) {
			// If found a uniform skip the type and extract the name
			uniformPosition += (keyword.length() + 1);
			std::size_t variableNameStart = shader.find(' ', uniformPosition);
			std::size_t variableNameEnd = shader.find(';', uniformPosition);
			++variableNameStart;
			std::string variableName = shader.substr(variableNameStart, variableNameEnd - variableNameStart);

			AddUniformLocation(variableName);

			// Find next uniform
			uniformPosition = shader.find(keyword, uniformPosition);
		}
	}
	
	glUseProgram(0);
}

void Shader::AddUniformLocation(const std::string& name) {
	GLint location = glGetUniformLocation(program, name.c_str());
	if (location < 0) {
		std::cout << "Warning: Failed to find uniform " << name << std::endl;
	}
	else {
		uniformLocations[name] = location;
	}
}

void Shader::Use() const {
	glUseProgram(program);
}


GLint Shader::GetLocation(const std::string& name) {
	if (uniformLocations.find(name) != uniformLocations.end()) {
		return uniformLocations.at(name);
	}
	else {
		return -1;
	}
	
}

void Shader::SetUniform(const std::string& name, GLint i) {
	glUniform1i(GetLocation(name), i);
}

void Shader::SetUniform(const std::string& name, GLfloat f) {
	glUniform1i(GetLocation(name), f);
}
void Shader::SetUniform(const std::string& name, const glm::mat4& mat) {
	glUniformMatrix4fv(GetLocation(name), 1, false, (float*) &mat);
}
void Shader::SetUniform(const std::string& name, const glm::vec3& vec) {
	glUniform3fv(GetLocation(name), 1, (float*)& vec);
}