#include "pch.h"
#include "glutils.h"

void SetMatrix4x4( const GLuint program, const GLfloat * data, const char * matrix_name ) {	
	const GLint location = glGetUniformLocation( program, matrix_name );

	if ( location == -1 ) {
		printf( "Matrix '%s' not found in active shader.\n", matrix_name );
	} else {
		glUniformMatrix4fv( location, 1, GL_TRUE, data );
	}
}

void SetVec3(const GLuint program, const GLfloat* data, const char* vecName) {
	const GLint location = glGetUniformLocation(program, vecName);

	if (location == -1) {
		printf("Vector '%s' not found in active shader.\n", vecName);
	} else {
		//glUniformMatrix4fv(location, 1, GL_TRUE, data);
		glUniform3fv(location, 1, data);
	}
}

void SetSampler(const GLuint program, GLenum texture_unit, const char* sampler_name) {
	const GLint location = glGetUniformLocation(program, sampler_name);

	if (location == -1) {
		printf("Texture sampler '%s' not found in active shader.\n", sampler_name);
	} else {
		glUniform1i(location, texture_unit);
	}
}
