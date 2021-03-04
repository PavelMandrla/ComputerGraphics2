﻿#include "pch.h"
#include "Rasterizer.h"
#include "utils.h"

char* Rasterizer::LoadShader(const char* file_name) {
	FILE * file = fopen( file_name, "rt" );

	if ( file == NULL ) {
		printf( "IO error: File '%s' not found.\n", file_name );
		return NULL;
	}

	size_t file_size = static_cast< size_t >( GetFileSize64( file_name ) );
	char * shader = NULL;

	if ( file_size < 1 ) {
		printf( "Shader error: File '%s' is empty.\n", file_name );
	} else {
		/* v glShaderSource nezadáváme v posledním parametru délku,
		takže øetìzec musí být null terminated, proto +1 a reset na 0*/
		shader = new char[file_size + 1];
		memset( shader, 0, sizeof( *shader ) * ( file_size + 1 ) );

		size_t bytes = 0; 

		do {
			bytes += fread( shader, sizeof( char ), file_size, file );
		} while ( !feof( file ) && ( bytes < file_size ) );

		if ( !feof( file ) && ( bytes != file_size ) ) {
			printf( "IO error: Unexpected end of file '%s' encountered.\n", file_name );
		}
	}

	fclose( file );
	file = NULL;

	return shader;
}

GLint Rasterizer::CheckShader(const GLenum shader) {
	GLint status = 0;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &status );

	printf( "Shader compilation %s.\n", ( status == GL_TRUE ) ? "was successful" : "FAILED" );

	if (status == GL_FALSE) {
		int info_length = 0;
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &info_length );
		char * info_log = new char[info_length];
		memset( info_log, 0, sizeof( *info_log ) * info_length );
		glGetShaderInfoLog( shader, info_length, &info_length, info_log );

		printf( "Error log: %s\n", info_log );

		SAFE_DELETE_ARRAY( info_log );
	}

	return status;
}

/* OpenGL check state */
bool Rasterizer::check_gl(const GLenum error) {
	if ( error != GL_NO_ERROR ) {
		//const GLubyte * error_str;
		//error_str = gluErrorString( error );
		//printf( "OpenGL error: %s\n", error_str );

		return false;
	}
	return true;
}

Rasterizer::Rasterizer(int width, int height, float fov, Vector3 viewFrom, Vector3 viewAt, float something, float somethingElse) {
	//TODO -> implement
}

Rasterizer::~Rasterizer() {
	delete this->window;
}

/* glfw callback */
void glfw_callback_1( const int error, const char * description ) {
	printf( "GLFW Error (%d): %s\n", error, description );
}

/* OpenGL messaging callback */
void GLAPIENTRY gl_callback_1( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void * user_param ) {
	printf( "GL %s type = 0x%x, severity = 0x%x, message = %s\n",
		( type == GL_DEBUG_TYPE_ERROR ? "Error" : "Message" ),
		type, severity, message );
}

/* invoked when window is resized */
void framebuffer_resize_callback_1( GLFWwindow * window, int width, int height ) {
	glViewport( 0, 0, width, height );
}

int Rasterizer::initDevice() {
	glfwSetErrorCallback( glfw_callback_1 );

	if (!glfwInit()) return( EXIT_FAILURE );

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
	glfwWindowHint( GLFW_SAMPLES, 8 );
	glfwWindowHint( GLFW_RESIZABLE, GL_TRUE );
	glfwWindowHint( GLFW_DOUBLEBUFFER, GL_TRUE );

	this->window = glfwCreateWindow( this->width, this->height, "PG2 OpenGL", nullptr, nullptr );
	if (!this->window) {
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwSetFramebufferSizeCallback( window, framebuffer_resize_callback_1 );
	glfwMakeContextCurrent( window );

	if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress )) {
		if (!gladLoadGL()) return EXIT_FAILURE;
	}

	glEnable( GL_DEBUG_OUTPUT );
	glDebugMessageCallback( gl_callback_1, nullptr );

	printf( "OpenGL %s, ", glGetString( GL_VERSION ) );
	printf( "%s", glGetString( GL_RENDERER ) );
	printf( " (%s)\n", glGetString( GL_VENDOR ) );
	printf( "GLSL %s\n", glGetString( GL_SHADING_LANGUAGE_VERSION ) );

	//check_gl();//wtf

	glEnable( GL_MULTISAMPLE );

	// map from the range of NDC coordinates <-1.0, 1.0>^2 to <0, width> x <0, height>
	glViewport( 0, 0, width, height );
	// GL_LOWER_LEFT (OpenGL) or GL_UPPER_LEFT (DirectX, Windows) and GL_NEGATIVE_ONE_TO_ONE or GL_ZERO_TO_ONE
	glClipControl( GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE );
}

void Rasterizer::initPrograms() {	///řeší vytvoření vertex a fragment shader
	// CREATE VERTEX SHADER
	this->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	const char * vertex_shader_source = LoadShader("basic_shader.vert");	//musime natahnout zdrojovy kod toho shaderu
	glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);		//nastavime mu zdrojovy soubor
	glCompileShader(vertex_shader);	
	SAFE_DELETE_ARRAY(vertex_shader_source);
	CheckShader(this->vertex_shader);

	// CREATE FRAGMENT SHADER
	this->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	const char * fragment_shader_source = LoadShader( "basic_shader.frag" );
	glShaderSource( this->fragment_shader, 1, &fragment_shader_source, nullptr );
	glCompileShader( this->fragment_shader );
	SAFE_DELETE_ARRAY( fragment_shader_source );
	CheckShader( this->fragment_shader );

	// CREATE PROGRAM
	this->shader_program = glCreateProgram();
	glAttachShader( shader_program, this->vertex_shader );
	glAttachShader( shader_program, this->fragment_shader );
	glLinkProgram( shader_program );
	
	// TODO check linking
	glUseProgram( shader_program );
}

void Rasterizer::loadScene(std::string &file_name) {
	//TODO -> fill this->verticies, this->no_verticies, this->indicies
}

void Rasterizer::initBuffers() {
	const int vertex_stride = sizeof( this->vertices ) / no_vertices;

	GLuint vao = 0; ///VAO BUFFER = popisuje konfiguraci bufferů, které budou v rámci objektu vystupovat
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao ); //timhle se ten buffer nabinduje k vao

	GLuint vbo = 0;	//vertex buffer
	glGenBuffers( 1, &vbo ); // generate vertex buffer object (one of OpenGL objects) and get the unique ID corresponding to that buffer
	glBindBuffer( GL_ARRAY_BUFFER, vbo ); // bind the newly created buffer to the GL_ARRAY_BUFFER target
	glBufferData( GL_ARRAY_BUFFER, sizeof( this->vertices ), this->vertices, GL_STATIC_DRAW ); // copies the previously defined vertex data into the buffer's memory	//prakticky memcpy, opengl neví nic o tom, jak jsou data zorganizovana
																				   // vertex position
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, vertex_stride, 0 );	//popiseme, jaka je struktura toho vertex bufferu = na nultem indexu jsou veci velikosti 3 (x, y, z) typu float a nechceme je normalizovat, stride je pocet bytu, ktere lezi mezi dvema nasledujicimi zaznamy | posledni je offset od zacatku pole
	glEnableVertexAttribArray( 0 );	//kazdy index, ktery popiseme, musime zenablovat
									// vertex texture coordinates		//popisujeme strukturu texturovacich souradnic
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, vertex_stride, ( void* )( sizeof( float ) * 3 ) );
	glEnableVertexAttribArray( 1 );

	GLuint ebo = 0; // optional buffer of indices			//element array buffer, pry to neni dobra cesta, nemusim ho vytvaret
	glGenBuffers( 1, &ebo );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ebo );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( this->indices ), this->indices, GL_STATIC_DRAW );
	//vbo a ebo ulozeno ve vao
	///po sem = ukázka, jak se tvoří vertex buffer

	
}
