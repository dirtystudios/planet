#include "MeshGeneration.h"
#include <vector>
#include <unordered_map>


namespace meshGen {

// Thanks bro...http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
IndexedMeshData CreateIcoSphere(uint32_t recursionLevel) {                
    IndexedMeshData meshData;
    int32_t index = 0;
    std::unordered_map<uint64_t, int32_t> middlePointIndexCache;

    // add vertex to mesh, fix position to be on unit sphere, return index
    auto addVertex = [&](const glm::vec3& p) -> int32_t{
        meshData.positions.push_back(glm::normalize(p));
        meshData.normals.push_back(glm::normalize(p));
        return index++;
    };

    // return index of point in the middle of p1 and p2
    auto getMiddlePoint = [&](int32_t p1, int32_t p2) -> int32_t {
        // first check if we have it already
        bool firstIsSmaller = p1 < p2;
        uint64_t smallerIndex = firstIsSmaller ? p1 : p2;
        uint64_t greaterIndex = firstIsSmaller ? p2 : p1;
        uint64_t key = (smallerIndex << 32) + greaterIndex;

        auto ret = middlePointIndexCache.find(key);
        if(ret != middlePointIndexCache.end()) {
            return ret->second;
        }
        
        // not in cache, calculate it
        glm::vec3 point1 = meshData.positions[p1];
        glm::vec3 point2 = meshData.positions[p2];
        glm::vec3 middle = glm::vec3(
            (point1.x + point2.x) / 2.f, 
            (point1.y + point2.y) / 2.f, 
            (point1.z + point2.z) / 2.f);

        // push_back vertex makes sure point is on unit sphere
        int i = addVertex(middle); 

        // store it, return index
        middlePointIndexCache.insert(std::make_pair(key, i));        
        return i;
    };

    // create 12 vertices of a icosahedron
    float t = (1.f + sqrt(5.f)) / 2.f;

    addVertex(glm::vec3(-1,  t,  0));
    addVertex(glm::vec3( 1,  t,  0));
    addVertex(glm::vec3(-1, -t,  0));
    addVertex(glm::vec3( 1, -t,  0));

    addVertex(glm::vec3( 0, -1,  t));
    addVertex(glm::vec3( 0,  1,  t));
    addVertex(glm::vec3( 0, -1, -t));
    addVertex(glm::vec3( 0,  1, -t));

    addVertex(glm::vec3( t,  0, -1));
    addVertex(glm::vec3( t,  0,  1));
    addVertex(glm::vec3(-t,  0, -1));
    addVertex(glm::vec3(-t,  0,  1));


    // create 20 triangles of the icosahedron
    std::vector<glm::ivec3> faces;

    // 5 faces around point 0
    faces.push_back(glm::ivec3(0, 11, 5));
    faces.push_back(glm::ivec3(0, 5, 1));
    faces.push_back(glm::ivec3(0, 1, 7));
    faces.push_back(glm::ivec3(0, 7, 10));
    faces.push_back(glm::ivec3(0, 10, 11));

    // 5 adjacent faces 
    faces.push_back(glm::ivec3(1, 5, 9));
    faces.push_back(glm::ivec3(5, 11, 4));
    faces.push_back(glm::ivec3(11, 10, 2));
    faces.push_back(glm::ivec3(10, 7, 6));
    faces.push_back(glm::ivec3(7, 1, 8));

    // 5 faces around point 3
    faces.push_back(glm::ivec3(3, 9, 4));
    faces.push_back(glm::ivec3(3, 4, 2));
    faces.push_back(glm::ivec3(3, 2, 6));
    faces.push_back(glm::ivec3(3, 6, 8));
    faces.push_back(glm::ivec3(3, 8, 9));

    // 5 adjacent faces 
    faces.push_back(glm::ivec3(4, 9, 5));
    faces.push_back(glm::ivec3(2, 4, 11));
    faces.push_back(glm::ivec3(6, 2, 10));
    faces.push_back(glm::ivec3(8, 6, 7));
    faces.push_back(glm::ivec3(9, 8, 1));

    // refine triangles
    for (uint32_t i = 0; i < recursionLevel; i++)
    {
        std::vector<glm::ivec3> faces2;
        for (const glm::ivec3& tri : faces) {
            // replace triangle by 4 triangles
            int a = getMiddlePoint(tri.x, tri.y);
            int b = getMiddlePoint(tri.y, tri.z);
            int c = getMiddlePoint(tri.z, tri.x);

            faces2.push_back(glm::ivec3(tri.x, a, c));
            faces2.push_back(glm::ivec3(tri.y, b, a));
            faces2.push_back(glm::ivec3(tri.z, c, b));
            faces2.push_back(glm::ivec3(a, b, c));
        }
        faces = faces2;
    }

    // done, now push_back triangles to mesh
    for(const glm::ivec3& tri : faces)
    {
        meshData.indices.push_back(tri.x);
        meshData.indices.push_back(tri.y);
        meshData.indices.push_back(tri.z);
    }

    return meshData;        
}

void GenerateGrid(float centerX, float centerY, float centerZ, float sizeX, float sizeY, uint32_t resolutionX, uint32_t resolutionY, VertexDelegate delegate) {
    float half_sizeX = sizeX / 2.f;
    float half_sizeY = sizeY / 2.f;

    float dx = sizeX / static_cast<float>(resolutionX - 1);
    float dy = sizeY / static_cast<float>(resolutionY - 1);

    auto generateVertex = [&](uint32_t i, uint32_t j) {
        float x         = centerX - half_sizeX + (j * dx);
        float y         = centerY + half_sizeY - (i * dy);
        float z         = centerZ;
        float u         = j * dx / sizeX;
        float v = i * dy / sizeY;
        delegate(x, y, z, u, v);
    };

    for (uint32_t i = 0; i < resolutionY - 1; ++i) {
        for (uint32_t j = 0; j < resolutionX - 1; ++j) {
            generateVertex(i, j);
            generateVertex(i + 1, j);
            generateVertex(i + 1, j + 1);
            generateVertex(i + 1, j + 1);
            generateVertex(i, j + 1);
            generateVertex(i, j);
        }
    }
}

void GenerateGrid(const glm::vec3& center, const glm::vec2& size, const glm::uvec2& resolution, VertexDelegate delegate) {
    GenerateGrid(center.x, center.y, center.z, size.x, size.y, resolution.x, resolution.y, delegate);
}

void GenerateCube(VertexDelegate delegate, float originX, float originY, float originZ, float scale, bool flip) {
    static float positions[6][6][3] = {
        {{-0.5, -0.5, 0.5},
         {0.5, -0.5, 0.5},
         {0.5, 0.5, 0.5},
         {0.5, 0.5, 0.5},
         {-0.5, 0.5, 0.5},
         {-0.5, -0.5, 0.5}}, // front

        {{0.5, -0.5, -0.5},
         {-0.5, -0.5, -0.5},
         {-0.5, 0.5, -0.5},
         {-0.5, 0.5, -0.5},
         {0.5, 0.5, -0.5},
         {0.5, -0.5, -0.5}}, // back

        {{-0.5, -0.5, -0.5},
         {-0.5, -0.5, 0.5},
         {-0.5, 0.5, 0.5},
         {-0.5, 0.5, 0.5},
         {-0.5, 0.5, -0.5},
         {-0.5, -0.5, -0.5}}, // left

        {{0.5, -0.5, 0.5},
         {0.5, -0.5, -0.5},
         {0.5, 0.5, -0.5},
         {0.5, 0.5, -0.5},
         {0.5, 0.5, 0.5},
         {0.5, -0.5, 0.5}}, // right

        {{-0.5, 0.5, 0.5},
         {0.5, 0.5, 0.5},
         {0.5, 0.5, -0.5},
         {0.5, 0.5, -0.5},
         {-0.5, 0.5, -0.5},
         {-0.5, 0.5, 0.5}}, // top

        {{0.5, -0.5, 0.5},
         {-0.5, -0.5, 0.5},
         {-0.5, -0.5, -0.5},
         {-0.5, -0.5, -0.5},
         {0.5, -0.5, -0.5},
         {0.5, -0.5, 0.5}} // bottom
    };

    static float texCoords[6][2] = {
        {0.f, 0.f }, //bl
        {1.f, 0.f }, //br
        {1.f, 1.f }, //tr
        {1.f, 1.f }, //tr
        {0.f, 1.f }, //tl
        {0.f, 0.f }  //bl
    };

    for (uint32_t f = 0; f < 6; ++f) { // for each face
        for (uint32_t v = 0; v < 6; ++v) { // for each vertex
            delegate(scale * positions[f][flip ? 5 - v : v][0] + originX, 
                     scale * positions[f][flip ? 5 - v : v][1] + originY,
                     scale * positions[f][flip ? 5 - v : v][2] + originZ,
                     texCoords[v][0],
                     texCoords[v][1]);            
        }
    }
}
}