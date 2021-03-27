#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec2 texcoord;

uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 mvn;

out vec3 unified_normal_es;

void main( void ) {

	unified_normal_es = normalize((mvn * vec4(normal.xyz, 0.0f)).xyz);
	vec4 hit_es = mv * vec4(position.xyz, 1.0f);
	vec3 omega_i_es = normalize( hit_es.xyz / hit_es.w );
	if (dot(unified_normal_es, omega_i_es) > 0.0f) {
		unified_normal_es *= -1.0f;
	}
	
	gl_Position = mvp * vec4( position, 1.0f );
}
