#include "MeshCache.h"
#include "File.h"

MeshCache::MeshCache(graphics::RenderDevice* device, const std::string& baseDir) : _device(device), _baseDir(baseDir) {
    if (!fs::IsPathDirectory(baseDir)) {
        LOG_E("%s", "Invalid Directory Path");
    }
}

MeshCache::~MeshCache() {
    // TODO: Destroy meshes
}

Mesh* MeshCache::Get(const std::string& name) {        
    auto it = _cache.find(name);
    if (it != _cache.end()) {
        return it->second;
    }

    string fpath = _baseDir + "/" + name;

    std::vector<IndexedMeshData> meshDatas = LoadMeshDataFromFile(fpath);
    assert(meshDatas.size() > 0);
	
   	
   	struct Vertex {
   		glm::vec3 pos;
   		glm::vec3 norm;
   	};

	std::vector<Vertex> vertices;		
	std::vector<uint32_t> indices;

	uint32_t offset = 0;
	for(const IndexedMeshData& meshData : meshDatas) {			
		assert(meshData.positions.size() == meshData.normals.size());

		for(uint32_t idx = 0; idx < meshData.positions.size(); ++idx) {					
			Vertex v;
			v.pos = meshData.positions[idx];
			v.norm =  meshData.normals[idx];
			vertices.push_back(v);
		}
		for(uint32_t idx = 0; idx < meshData.indices.size(); ++idx) {				
			indices.push_back(offset + meshData.indices[idx]);
		}
		offset += meshData.positions.size();
	}		
	graphics::BufferId vb = _device->CreateBuffer(graphics::BufferType::VertexBuffer, vertices.data(), sizeof(Vertex) * vertices.size(), graphics::BufferUsage::Static);
	graphics::BufferId ib = _device->CreateBuffer(graphics::BufferType::IndexBuffer, indices.data(), sizeof(uint32_t) * indices.size(), graphics::BufferUsage::Static);
	assert(vb);
	assert(ib);
	Mesh* mesh = new Mesh();
	mesh->vertexBuffer = vb;
	mesh->indexBuffer = ib;
	mesh->indexCount = indices.size();
	mesh->indexOffset = 0;

    _cache.insert(make_pair(name, mesh));
    return mesh;
}