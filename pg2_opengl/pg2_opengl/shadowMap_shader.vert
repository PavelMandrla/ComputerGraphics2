#version 460 core

layout ( location = 0 ) in vec4 in_position_ms;

uniform mat4 mlp;

void main( void ) {
	gl_Position= mlp * in_position_ms;
}