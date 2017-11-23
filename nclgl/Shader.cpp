#include "Shader.h"

#include <iostream>
#include <sstream>

#include "Light.h" // Just including this for MAX_LIGHTS...

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
		__debugbreak();
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
		__debugbreak();
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
		__debugbreak();
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
	glBindAttribLocation(program, TANGENT_BUFFER, "tangent");
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
	if (name.find('[') != std::string::npos) {// Is it an array?
		AddArrayUniformLocation(name);
	}
	else {
		GLint location = glGetUniformLocation(program, name.c_str());
		if (location < 0) {
			std::cout << "Warning: Failed to find uniform " << name << std::endl;
		}
		else {
			uniformLocations[name] = location;
		}
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

// Struct uniforms are a little bit awkward. 
// To save time will just be really ugly and hardcode them for now 
void Shader::AddArrayUniformLocation(const std::string& name) {
	if (name.find("point") != std::string::npos) {
		AddPointLightUniformLocation();
	}
	else if (name.find("direction") != std::string::npos) {
		AddDirLightUniformLocation();
	}
	else if (name.find("spot") != std::string::npos) {
		AddSpotLightUniformLocation();
	}
	else if (name.find("lightViewMat") != std::string::npos) {
		AddLightViewUniformLocation();
	}
	else {
		std::cout << "Warning: Failed to find uniform " << name << std::endl;
	}


}


std::vector<std::string> Shader::GetUniformNames() {
	std::vector<std::string> names;

	for (auto const& uniform : uniformLocations)
		names.push_back(uniform.first);

	return names;
}

GLint Shader::SetUniform(const std::string& name, GLint i) {
	glUniform1i(GetLocation(name), i);
	return GetLocation(name);
}

GLint Shader::SetUniform(const std::string& name, GLfloat f) {
	glUniform1f(GetLocation(name), f);
	return GetLocation(name);
}
GLint Shader::SetUniform(const std::string& name, const glm::mat4& mat) {
	glUniformMatrix4fv(GetLocation(name), 1, false, (float*) &mat);
	return GetLocation(name);
}
GLint Shader::SetUniform(const std::string& name, const glm::vec3& vec) {
	glUniform3fv(GetLocation(name), 1, (float*)& vec);
	return GetLocation(name);
}

GLint Shader::SetUniform(const std::string& name, const glm::vec2& vec) {
	glUniform2fv(GetLocation(name), 1, (float*)& vec);
	return GetLocation(name);
}

GLint Shader::SetUniform(const std::string& name, const glm::vec4& vec) {
	glUniform4fv(GetLocation(name), 1, (float*)& vec);
	return GetLocation(name);
}

// Struct uniforms are a little bit awkward. 
// To save time will just be really ugly and hardcode them for now 
void Shader::AddPointLightUniformLocation() {
	for (int i = 0; i < MAX_LIGHTS; ++i) {
		std::stringstream ss;
		ss << i;
		std::string name = "pointLights[" + ss.str() + "].position";
		GLint location = glGetUniformLocation(program, name.c_str());
		if (location < 0) {
			std::cout << "Warning: Failed to find uniform " << name << std::endl;
		}
		else {
			uniformLocations[name] = location;
		}
		name = "pointLights[" + ss.str() + "].colour";
		location = glGetUniformLocation(program, name.c_str());
		if (location < 0) {
			std::cout << "Warning: Failed to find uniform " << name << std::endl;
		}
		else {
			uniformLocations[name] = location;
		}
		name = "pointLights[" + ss.str() + "].radius";
		location = glGetUniformLocation(program, name.c_str());
		if (location < 0) {
			std::cout << "Warning: Failed to find uniform " << name << std::endl;
		}
		else {
			uniformLocations[name] = location;
		}
	}
}

// Struct uniforms are a little bit awkward. 
// To save time will just be really ugly and hardcode them for now 
void Shader::AddDirLightUniformLocation() {
	const int MAX_DIR_LIGHTS = 1; // TODO: Make a namespace
	for (int i = 0; i < MAX_DIR_LIGHTS; ++i) {
		std::stringstream ss;
		ss << i;
		std::string name = "directionalLights[" + ss.str() + "].direction";
		GLint location = glGetUniformLocation(program, name.c_str());
		if (location < 0) {
			std::cout << "Warning: Failed to find uniform " << name << std::endl;
		}
		else {
			uniformLocations[name] = location;
		}
		name = "directionalLights[" + ss.str() + "].colour";
		location = glGetUniformLocation(program, name.c_str());
		if (location < 0) {
			std::cout << "Warning: Failed to find uniform " << name << std::endl;
		}
		else {
			uniformLocations[name] = location;
		}
	}
}

void Shader::AddSpotLightUniformLocation() {
	const int MAX_SPOTLIGHTS = 1; // TODO: Make a namespace
	for (int i = 0; i < MAX_SPOTLIGHTS; ++i) {
		std::stringstream ss;
		ss << i;
		std::string name = "spotLights[" + ss.str() + "].direction";
		GLint location = glGetUniformLocation(program, name.c_str());
		if (location < 0) {
			std::cout << "Warning: Failed to find uniform " << name << std::endl;
		}
		else {
			uniformLocations[name] = location;
		}
		name = "spotLights[" + ss.str() + "].colour";
		location = glGetUniformLocation(program, name.c_str());
		if (location < 0) {
			std::cout << "Warning: Failed to find uniform " << name << std::endl;
		}
		else {
			uniformLocations[name] = location;
		}
		name = "spotLights[" + ss.str() + "].radius";
		location = glGetUniformLocation(program, name.c_str());
		if (location < 0) {
			std::cout << "Warning: Failed to find uniform " << name << std::endl;
		}
		else {
			uniformLocations[name] = location;
		}
		name = "spotLights[" + ss.str() + "].innerCutOff";
		location = glGetUniformLocation(program, name.c_str());
		if (location < 0) {
			std::cout << "Warning: Failed to find uniform " << name << std::endl;
		}
		else {
			uniformLocations[name] = location;
		}
		name = "spotLights[" + ss.str() + "].outerCutOff";
		location = glGetUniformLocation(program, name.c_str());
		if (location < 0) {
			std::cout << "Warning: Failed to find uniform " << name << std::endl;
		}
		else {
			uniformLocations[name] = location;
		}
		name = "spotLights[" + ss.str() + "].position";
		location = glGetUniformLocation(program, name.c_str());
		if (location < 0) {
			std::cout << "Warning: Failed to find uniform " << name << std::endl;
		}
		else {
			uniformLocations[name] = location;
		}
	}
}

void Shader::AddLightViewUniformLocation() {
	const int NUM_FACES = 6;
	for (int i = 0; i < NUM_FACES; ++i) {
		std::string name = "lightViewMatrices[" + std::to_string(i) + "]";
		GLint location = glGetUniformLocation(program, name.c_str());
		if (location < 0) {
			std::cout << "Warning: Failed to find uniform " << name << std::endl;
		}
		else {
			uniformLocations[name] = location;
		}
	}
}