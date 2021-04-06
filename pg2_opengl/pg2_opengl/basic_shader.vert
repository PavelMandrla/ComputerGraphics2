#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec2 texcoord;

uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 mvn;
uniform mat4 mlp;

uniform vec3 cameraDir;

out vec3 v_normal;
out vec3 unified_normal_es;
out vec3 position_lcs;
out vec3 v_cameraDir;

void main( void ) {
	v_normal = normalize(normal);
	v_cameraDir = cameraDir;

	unified_normal_es = normalize((mvn * vec4(normal.xyz, 0.0f)).xyz);
	vec4 hit_es = mv * vec4(position.xyz, 1.0f);
	vec3 omega_i_es = normalize( hit_es.xyz / hit_es.w );
	if (dot(unified_normal_es, omega_i_es) > 0.0f) {
		unified_normal_es *= -1.0f;
	}
	
	gl_Position = mvp * vec4( position, 1.0f );

	vec4 tmp = mlp * vec4( position.xyz, 1.0f );
	position_lcs = tmp.xyz / tmp.w;
}
