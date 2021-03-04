#pragma once

#include "vector3.h"

class Rasterizer {
private:
	int width, height;

	GLfloat* vertices; //GLfloat vertices[];
	int no_vertices;
	unsigned int* indices; //unsigned int indices[];

	//void glfw_callback(const int error, const char* description);

	char* LoadShader(const char* file_name);
		
	GLint CheckShader(const GLenum shader);

	bool check_gl(const GLenum error);

	//INIT DEVICE
	GLFWwindow* window;

	//INIT PROGRAMS
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint shader_program;

	//INIT BUFFERS



public:
	Rasterizer(int width, int height, float fov, Vector3 viewFrom, Vector3 viewAt, float something, float somethingElse); //TODO -> fix something and somethingElse params

	~Rasterizer();

	/// <summary>
	/// Initialization of OpenGL context
	/// </summary>
	/// <returns></returns>
	int initDevice();

	/// <summary>
	/// Initialization of VS and FS shaders
	/// </summary>
	void initPrograms();

	/// <summary>
	/// Loads the scene geometry
	/// </summary>
	void loadScene(std::string &file_name);

	/// <summary>
	/// Initialization of VAO and VBO buffers
	/// </summary>
	void initBuffers();

};

