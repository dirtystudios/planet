#include "MeshCache.h"
#include "File.h"
#include "MeshImporter.h"
#include "MeshGeneration.h"


MeshCache::MeshCache(graphics::RenderDevice* device, const std::string& baseDir) : _device(device), _baseDir(baseDir) {
    if (!fs::IsPathDirectory(baseDir)) {
        LOG_E("%s", "Invalid Directory Path");
        _baseDir = fs::AppendPathProcessDir("/assets");
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
    std::vector<IndexedMeshData> meshDatas = meshImport::LoadMeshDataFromFile(fpath);
    assert(meshDatas.size() > 0);
    
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 norm;
        glm::vec2 texCoord;
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
            v.texCoord = meshData.texcoords[idx];
            vertices.push_back(v);
        }
        for(uint32_t idx = 0; idx < meshData.indices.size(); ++idx) {				
            indices.push_back(offset + meshData.indices[idx]);
        }

        offset += meshData.positions.size();
    }		
    size_t verticesSize = sizeof(Vertex) * vertices.size();
    size_t indicesSize = sizeof(uint32_t) * indices.size();
    graphics::BufferId vb = _device->AllocateBuffer(graphics::BufferType::VertexBuffer, verticesSize, graphics::BufferUsage::Static);
    graphics::BufferId ib = _device->AllocateBuffer(graphics::BufferType::IndexBuffer, indicesSize, graphics::BufferUsage::Static);
    assert(ib);
    assert(vb);
    
    
    uint8_t* ptr = _device->MapMemory(vb, graphics::BufferAccess::Write);
    memcpy(ptr, vertices.data(), verticesSize);
    _device->UnmapMemory(vb);
    
    ptr = _device->MapMemory(ib, graphics::BufferAccess::Write);
    memcpy(ptr, indices.data(), indicesSize);
    _device->UnmapMemory(ib);
    
    Mesh* mesh = new Mesh();
    mesh->vertexBuffer = vb;
    mesh->indexBuffer = ib;
    mesh->indexCount = indices.size();
    mesh->indexOffset = 0;
    mesh->vertexStride = sizeof(Vertex);
    mesh->vertexOffset = 0;

    _cache.insert(make_pair(name, mesh));
    return mesh;
}