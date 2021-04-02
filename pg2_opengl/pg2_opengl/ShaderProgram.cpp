#include "pch.h"
#include "ShaderProgram.h"
#include "utils.h"

ShaderProgram::ShaderProgram(string vertPath, string fragPath) {
	this->initPrograms(vertPath, fragPath);
}

ShaderProgram::~ShaderProgram() {
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(program);
}

char* ShaderProgram::LoadShader(const char* file_name) {
	FILE* file = fopen(file_name, "rt");

	if (file == NULL) {
		printf("IO error: File '%s' not found.\n", file_name);
		return NULL;
	}

	size_t file_size = static_cast<size_t>(GetFileSize64(file_name));
	char* shader = NULL;

	if (file_size < 1) {
		printf("Shader error: File '%s' is empty.\n", file_name);
	} else {
		/* v glShaderSource nezadáváme v posledním parametru délku,
		takže øetìzec musí být null terminated, proto +1 a reset na 0*/
		shader = new char[file_size + 1];
		memset(shader, 0, sizeof(*shader) * (file_size + 1));

		size_t bytes = 0;

		do {
			bytes += fread(shader, sizeof(char), file_size, file);
		} while (!feof(file) && (bytes < file_size));

		if (!feof(file) && (bytes != file_size)) {
			printf("IO error: Unexpected end of file '%s' encountered.\n", file_name);
		}
	}

	fclose(file);
	file = NULL;

	return shader;
}

GLint ShaderProgram::CheckShader(const GLenum shader) {
	GLint status = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	printf("Shader compilation %s.\n", (status == GL_TRUE) ? "was successful" : "FAILED");

	if (status == GL_FALSE) {
		int info_length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_length);
		char* info_log = new char[info_length];
		memset(info_log, 0, sizeof(*info_log) * info_length);
		glGetShaderInfoLog(shader, info_length, &info_length, info_log);

		printf("Error log: %s\n", info_log);

		SAFE_DELETE_ARRAY(info_log);
	}

	return status;
}

void ShaderProgram::initPrograms(string vertPath, string fragPath) {
	// CREATE VERTEX SHADER
	this->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	const char* vertex_shader_source = LoadShader(vertPath.c_str());	//musime natahnout zdrojovy kod toho shaderu
	glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);		//nastavime mu zdrojovy soubor
	glCompileShader(vertex_shader);
	SAFE_DELETE_ARRAY(vertex_shader_source);
	CheckShader(this->vertex_shader);

	// CREATE FRAGMENT SHADER
	this->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	const char* fragment_shader_source = LoadShader(fragPath.c_str());
	glShaderSource(this->fragment_shader, 1, &fragment_shader_source, nullptr);
	glCompileShader(this->fragment_shader);
	SAFE_DELETE_ARRAY(fragment_shader_source);
	CheckShader(this->fragment_shader);

	// CREATE PROGRAM
	this->program = glCreateProgram();
	glAttachShader(program, this->vertex_shader);
	glAttachShader(program, this->fragment_shader);
	glLinkProgram(program);

	GLint program_linked;
	glGetProgramiv(program, GL_LINK_STATUS, &program_linked);
	if (program_linked != GL_TRUE) {
		GLsizei log_length = 0;
		GLchar message[1024];
		glGetProgramInfoLog(program, 1024, &log_length, message);
		printf(message);
		// Write the error to a log
	}
}
