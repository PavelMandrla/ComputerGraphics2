#include "pch.h"
#include "Scene.h"
#include "objloader.h"

Scene::Scene(std::string& file_name) {
	const int no_surfaces = LoadOBJ( file_name.c_str(), surfaces_, materials_ );

	for ( auto surface : surfaces_ ) {
		for ( int i = 0, k = 0; i < surface->no_triangles(); i++) {
			Triangle & triangle = surface->get_triangle(i);
			
			for (int j = 0; j < 3; j++, k += 3) {
				const Vertex & vertex = triangle.vertex(j);
				verticies.push_back(MyVertex {
					vertex.position.x,
					vertex.position.y,
					vertex.position.z,
					vertex.texture_coords->u,
					vertex.texture_coords->v
					});
			}
			
		}
	}

	this->vertex_stride = sizeof(MyVertex);
}
