#include "pch.h"
#include "Scene.h"
#include "objloader.h"

Scene::Scene(std::string& file_name, std::string& background_file) {
	this->background = std::make_shared<Texture3f>(background_file);

	const int no_surfaces = LoadOBJ( file_name.c_str(), surfaces_, materials_ );

	for ( auto surface : surfaces_ ) {
		for ( int i = 0, k = 0; i < surface->no_triangles(); i++) {
			Triangle & triangle = surface->get_triangle(i);
			
			for (int j = 0; j < 3; j++, k += 3) {
				const Vertex & vertex = triangle.vertex(j);
				//vertex.normal

				verticies.push_back(MyVertex {
					// position
					vertex.position.x,
					vertex.position.y,
					vertex.position.z,
					// normal
					vertex.normal.x,
					vertex.normal.y,
					vertex.normal.z,
					// tangent
					vertex.tangent.x,
					vertex.tangent.y,
					vertex.tangent.z,
					// texture coordinates
					vertex.texture_coords->u,
					vertex.texture_coords->v
					});
			}
			
		}
	}

	this->vertex_stride = sizeof(MyVertex);
}

Texture3f Scene::getIrradianceMap(int width, int height) {
	Texture3f result(width, height);

	for (int x = 0; x < result.width(); x++) {
		float phi = float(x) * 2.0f * M_PI / float(result.width());
		for (int y = 0; y < result.height(); y++) {
			float theta = float(y) * M_PI / float(result.height());

			float u = phi / (2 * M_PI);
			float v = theta / (M_PI);

			result.data()[size_t(x) + size_t(y) * size_t(result.width())] = this->background->texel(u, v);
		}
	}
	//result.Save("D:\\prg\\cpp\\save_test.exr");
	
	return result;
}
