#pragma once
#include <glad/glad.h>
#include <string>

using namespace std;

class ShaderProgram {
public:
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint program;

	ShaderProgram(string vertPath, string fragPath);
	~ShaderProgram();

	char* LoadShader(const char* file_name);
	GLint CheckShader(const GLenum shader);
	void initPrograms(string vertPath, string fragPath);
};

