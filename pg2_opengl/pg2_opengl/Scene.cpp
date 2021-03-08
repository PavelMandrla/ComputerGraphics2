#include "pch.h"
#include "Scene.h"
#include "objloader.h"

Scene::Scene(std::string& file_name) {
	const int no_surfaces = LoadOBJ( file_name.c_str(), surfaces_, materials_ );
	this->no_vertices = 0;

	for ( auto surface : surfaces_ ) {
		for ( int i = 0, k = 0; i < surface->no_triangles(); i++) {
			Triangle & triangle = surface->get_triangle(i);
			//p0.x, p0.y, p0.z, t0.u, t0.v
			for (int j = 0; j < 3; j++, k += 3) {
				const Vertex & vertex = triangle.vertex(j);
				verticies.push_back(vertex.position.x);
				verticies.push_back(vertex.position.y);
				verticies.push_back(vertex.position.z);
				verticies.push_back(vertex.texture_coords->u);
				verticies.push_back(vertex.texture_coords->v);
				no_vertices++;
			}
			indices.push_back(k);
			indices.push_back(k+1);
			indices.push_back(k+2);
		}
	}

	this->vertex_stride = this->verticies.size() / no_vertices;
}
