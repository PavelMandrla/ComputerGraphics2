#include "pch.h"
#include "Rasterizer.h"
#include "utils.h"
#include "glutils.h"
#include "mymath.h"

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

// OpenGL check state
bool Rasterizer::check_gl(const GLenum error) {
	if ( error != GL_NO_ERROR ) {
		//const GLubyte * error_str;
		//error_str = gluErrorString( error );
		//printf( "OpenGL error: %s\n", error_str );

		return false;
	}
	return true;
} 

// glfw callback
void glfw_callback_1( const int error, const char * description ) {
	printf( "GLFW Error (%d): %s\n", error, description );
}

// OpenGL messaging callback
void GLAPIENTRY gl_callback_1( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void * user_param ) {
	printf( "GL %s type = 0x%x, severity = 0x%x, message = %s\n",
		( type == GL_DEBUG_TYPE_ERROR ? "Error" : "Message" ),
		type, severity, message );
}

// invoked when window is resized
void framebuffer_resize_callback_1( GLFWwindow * window, int width, int height ) {
	glViewport( 0, 0, width, height );
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		auto rasterizer = reinterpret_cast<Rasterizer*>(glfwGetWindowUserPointer(window));
		auto camera = rasterizer->getCamera();
		if (key == GLFW_KEY_W) camera->moveForward();
		if (key == GLFW_KEY_S) camera->moveBackward();
		if (key == GLFW_KEY_A) camera->moveLeft();
		if (key == GLFW_KEY_D) camera->moveRight();
	}

}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	//printf("x:%f y:%f\n", xpos, ypos);
	auto camera = reinterpret_cast<Rasterizer*>(glfwGetWindowUserPointer(window))->getCamera();
	camera->adjustYaw(ypos);
	camera->adjustPitch(xpos);

	glfwSetCursorPos(window, 0, 0);
}

Rasterizer::Rasterizer(int width, int height, float fovY, Vector3 viewFrom, Vector3 viewAt) {
	this->camera = std::make_shared<Camera>(width, height, fovY, viewFrom, viewAt);
}

Rasterizer::~Rasterizer() {

	glDeleteShader( vertex_shader );
	glDeleteShader( fragment_shader );
	glDeleteProgram( shader_program );

	glDeleteBuffers( 1, &vbo );
	glDeleteVertexArrays( 1, &vao );

	glfwTerminate();
	delete this->window;
}

void Rasterizer::generateIrradianceMap() {
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

	this->window = glfwCreateWindow( camera->getWidth(), camera->getHeight(), "PG2 OpenGL", nullptr, nullptr );
	if (!this->window) {
		glfwTerminate();
		return EXIT_FAILURE;
	}
	glfwSetWindowUserPointer(this->window, reinterpret_cast<void*>(this));
	glfwSetKeyCallback(this->window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
	glViewport(0, 0, camera->getWidth(), camera->getHeight());
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


	GLint program_linked;
	glGetProgramiv(shader_program, GL_LINK_STATUS, &program_linked);
	if (program_linked != GL_TRUE) {
		GLsizei log_length = 0;
		GLchar message[1024];
		glGetProgramInfoLog(shader_program, 1024, &log_length, message);
		printf(message);
		// Write the error to a log
	}
	
	// TODO check linking
	glUseProgram( shader_program );

	//TODO - asi remove
	//glPointSize( 10.0f );	
	//glLineWidth( 2.0f );
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}

void Rasterizer::loadScene(std::string file_name, std::string background_file) {
	this->scene = std::make_shared<Scene>(file_name, background_file);
}

void Rasterizer::initBuffers() {	
	this->vao = 0; ///VAO BUFFER = popisuje konfiguraci bufferů, které budou v rámci objektu vystupovat
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao ); //timhle se ten buffer nabinduje k vao

	this->vbo = 0;	//vertex buffer
	glGenBuffers( 1, &vbo ); // generate vertex buffer object (one of OpenGL objects) and get the unique ID corresponding to that buffer
	glBindBuffer( GL_ARRAY_BUFFER, vbo ); // bind the newly created buffer to the GL_ARRAY_BUFFER target

	auto vertices = this->scene->getVerticies();
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(MyVertex), vertices.data(), GL_STATIC_DRAW);
	
	// vertex position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, scene->getVertexStride(), 0);	//popiseme, jaka je struktura toho vertex bufferu = na nultem indexu jsou veci velikosti 3 (x, y, z) typu float a nechceme je normalizovat, stride je pocet bytu, ktere lezi mezi dvema nasledujicimi zaznamy | posledni je offset od zacatku pole
	glEnableVertexAttribArray(0);	

	// vertex position
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, scene->getVertexStride(), (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, scene->getVertexStride(), (void*)(sizeof(float) * 6));
	glEnableVertexAttribArray(2);
	
	// vertex texture coordinates		
	glVertexAttribPointer( 3, 2, GL_FLOAT, GL_FALSE, scene->getVertexStride(), (void*) (sizeof(float) * 9));
	glEnableVertexAttribArray(3); //kazdy index, ktery popiseme, musime zenablovat
	
}

void Rasterizer::mainLoop() {
	glEnable( GL_DEPTH_TEST ); // zrusi pouziti z-bufferu, vykresleni se provede bez ohledu na poradi fragmentu z hlediska jejich pseudohloubky
	glEnable( GL_CULL_FACE ); // zrusi zahazovani opacne orientovanych ploch

	this->scene->getIrradianceMap(64, 32);
	
	while (!glfwWindowShouldClose(this->window)) {		
		glClearColor( 0.2f, 0.3f, 0.3f, 1.0f ); // state setting function
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT ); // state using function

		glBindVertexArray( vao );

		SetMatrix4x4(this->shader_program, (GLfloat*) camera->getMVP().data(), "mvp");
		SetMatrix4x4(this->shader_program, (GLfloat*)camera->getMV().data(), "mv");
		SetMatrix4x4(this->shader_program, (GLfloat*)camera->getMVn().data(), "mvn");

		glDrawArrays( GL_TRIANGLES, 0, this->scene->getVerticies().size() ); //??
		//glDrawArrays( GL_POINTS, 0, 6 );
		//glDrawArrays( GL_LINE_LOOP, 0, this->scene->getVerticies().size() );
		//glDrawElements( GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0 ); // optional - render from an index buffer

		glfwSwapBuffers( window );
		glfwPollEvents();
	}
}
