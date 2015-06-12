#ifndef __terrain_manager_h__
#define __terrain_manager_h__

#include <queue>
#include <map>
#include <vector>
#include <list>
#include <glm/glm.hpp>
#include "GLHelpers.h"
#include <cassert>
#include <unordered_map>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <noise/noise.h> 
namespace t {
#define RES 16

struct Camera {
    Camera(float fov_degrees = 45.f, float aspect_ratio = 1.33333f, float znear = 0.1f, float zfar = 10000.f);

    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 look;
    glm::dvec3 pos;

    float fov_degrees;
    double zfar;
    double znear;
    float aspect_ratio;

    void Translate(glm::vec3 translation);
    void MoveTo(glm::vec3 pos);
    void Yaw(float degrees);
    void Pitch(float degrees);
    void LookAt(glm::vec3 target);
    
    float GetHorizontalFieldOfView() const {
        return fov_degrees;
    }
    float GetVerticalFieldOfView();
    glm::mat4 GetView() const;
    glm::mat4 GetProjection() const;    
};


struct BoundingBox {
    BoundingBox(glm::dvec3 min, glm::dvec3 max) : min(min), max(max) {};
    BoundingBox() {}
    
    glm::dvec3 min;
    glm::dvec3 max;    
};

struct Frustum {
    bool BoxInFrustum(const BoundingBox& bbox) const {
        return true;
    };
};


double DistanceFromBoundingBox(const BoundingBox& bbox, const glm::dvec3& p) {
    glm::dvec3 nearest_point_on_bbox = glm::dvec3();    
    nearest_point_on_bbox.x = (p.x < bbox.min.x) ? bbox.min.x : (p.x > bbox.max.x) ? bbox.max.x : p.x;
    nearest_point_on_bbox.y = (p.y < bbox.min.y) ? bbox.min.y : (p.y > bbox.max.y) ? bbox.max.y : p.y;
    nearest_point_on_bbox.z = (p.z < bbox.min.z) ? bbox.min.z : ((p.z > bbox.max.z) ? bbox.max.z : p.z);   
    if(nearest_point_on_bbox == p) {
        return 0; // inside/on the bbox
    } else {
        return glm::length(p - nearest_point_on_bbox);    
    }    
}


double ComputeScreenSpaceError(const BoundingBox& bbox, const glm::dvec3& eye, float hfov, uint32_t viewport_width, float geometric_error) {
    double D = DistanceFromBoundingBox(bbox, eye);            
    double w = 2.f * D * tan(hfov / 2.f);
    if(w == 0) {
        return 0; //TODO: this mean if we are ever inside the bounding box, we say we have 0 screen space error. NOT good.
    }
    double res = geometric_error * viewport_width / w;        
    return res;   
}

struct HeightmapRegionData {
    std::vector<float> data;
    glm::dvec3 world;
    double size;
    float zmin;
    float zmax;
    uint32_t resolution;
};

struct HeightmapGenerator {
    uint32_t resolution = RES;
    double scale = 0.1;
    void GenerateRegion(const glm::dvec3& world, double size, std::vector<float>* data, float* zmin, float* zmax) {
        noise::module::Perlin perlin;
        perlin.SetSeed(0);
        perlin.SetFrequency(0.5);
        perlin.SetOctaveCount(8);
        perlin.SetPersistence(0.25);
        double half_size = size / 2.f;
        float dx = size / (float)(resolution - 1);
        float dy = size / (float)(resolution - 1);
        *zmin = 99999999;
        *zmax =-99999999;
        for(uint32_t i = 0; i < resolution; ++i) {
            for(uint32_t j = 0; j < resolution; ++j) {
                float x = world.x - half_size + (j * dx);
                float y = world.y + half_size - (i * dy);
                double val = perlin.GetValue(x * scale, y * scale, 0) * 50;
                data->push_back(val);
                if(val > *zmax) {
                    *zmax = val;
                } else if(val < *zmin) {
                    *zmin = val;
                }
            }
        }
    }



};

struct TerrainNode {
    BoundingBox bbox;
    
    double size;
    glm::dvec3 world;

    uint32_t tx;
    uint32_t ty;
    uint32_t lod_level;
        
    float geometric_error;

    TerrainNode* children { NULL };
};

struct Terrain {
    uint32_t max_lod;
    glm::dvec3 world;
    double size;
    double tau;
    uint32_t resolution;

    TerrainNode* root { NULL };

    HeightmapGenerator heightmap_generator;
};


struct Viewport {
    uint32_t width;
    uint32_t height;
};



 

struct GPUTile {
    GPUTile() {};
    GPUTile(GLuint tex_id, uint32_t idx) : texture_array_id(tex_id), index(idx) {};

    GLuint texture_array_id;
    uint32_t index;

    void CopyData(uint32_t width, uint32_t height, void* data) { 
        gl::UpdateTexture2DArray(texture_array_id, GL_RED, GL_FLOAT, width, height, index, data);
    }
};

struct GPUTileBuffer {
    uint32_t tile_size;
    uint32_t num_tiles;
    GLuint tex_id;
    GPUTile* tiles;
    
    std::list<GPUTile*> unused;
    std::list<GPUTile*> used;

    GPUTileBuffer(uint32_t tile_size, uint32_t num_tiles) {
        this->tile_size = tile_size;
        this->num_tiles = num_tiles;
        tex_id = gl::CreateTexture2DArray(GL_R32F, 1, tile_size, tile_size, num_tiles);    
        tiles = new GPUTile[num_tiles];
        for(uint32_t i = 0; i < num_tiles; ++i) {
            tiles[i].texture_array_id = tex_id;
            tiles[i].index = i;
            unused.push_back(&tiles[i]);
        }
    }

    GPUTile* GetFreeTile() {
        GPUTile* tile = unused.front();
        if(tile) {
            unused.remove(tile);
            used.push_back(tile);
            printf("GPUTileBuffer:  used:%d unused:%d\n", used.size(), unused.size()); 
        }
        return tile; 
    }

    void FreeTile(GPUTile* tile) {
        used.remove(tile);
        unused.push_front(tile);
    }

    uint32_t GetMaxCapacity() {
        return num_tiles;
    }

    uint32_t GetUnusedCount() {
        return unused.size();
    }
};

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

struct GPUTileCache {

    size_t GetTileId(uint32_t lod, uint32_t tx, uint32_t ty) {
        size_t seed = 0;
        hash_combine(seed, lod);
        hash_combine(seed, tx);
        hash_combine(seed, ty);
        return seed;
    } 
    struct Tile {
        Tile(uint32_t lod, uint32_t tx, uint32_t ty, GPUTile* data) : lod(lod), tx(tx), ty(ty), data(data) {};
        
        uint32_t lod;
        uint32_t tx;
        uint32_t ty;
        GPUTile* data;
    };
    
    GPUTileBuffer* tile_buffer;
    std::list<Tile*> lru;
    std::map<size_t, Tile*> cache;
    

    GPUTileCache(GPUTileBuffer* tile_buffer) {
        this->tile_buffer = tile_buffer; 
    };

    ~GPUTileCache() {
        for(std::pair<size_t, Tile*> p : cache) {
            delete p.second;
        }
    }
    
    void print() {
        //printf("(%zu), ", lru.front()->lod, lru.front()->tx, lru.front()->ty);
        //printf("(%zu), ", lru.back()->lod, lru.back()->tx, lru.back()->ty);
        //printf("\n");
    }

    Tile* GetTile(uint32_t lod, uint32_t tx, uint32_t ty, std::function<void(GPUTile* tile)> func) {
        size_t id = GetTileId(lod, tx, ty);
        std::map<size_t, Tile*>::iterator it = cache.find(id);
        Tile* tile = NULL;
        if(it != cache.end()) {            
            tile = it->second;
            lru.remove(tile);
        } else {
            GPUTile* gpu_tile = tile_buffer->GetFreeTile();
            if(gpu_tile != NULL) {
                func(gpu_tile);
                tile = new Tile(lod, tx, ty, gpu_tile);
                cache.insert(std::make_pair(id, tile));                    
            } else {
                // buffer is full, need to evict
                tile = lru.back();
                printf("Buffer is full, evicting [%d %d %d] for [%d %d %d]\n", tile->lod, tile->tx, tile->ty, lod, tx, ty);
                assert(tile);
                lru.remove(tile);
                func(tile->data);
                it = cache.find(GetTileId(tile->lod, tile->tx, tile->ty));
                assert(it != cache.end());
                cache.erase(it);
                tile->lod = lod;
                tile->tx = tx;
                tile->ty = ty;
                cache.insert(std::make_pair(id, tile));
            }
        }   
        lru.push_front(tile);
        print();
        return tile;
        
    }  
};


struct TerrainRenderer {
        
    GPUTileCache* gpu_tile_cache;
    GPUTileBuffer* gpu_tile_buffer;
    struct Vertex {
        Vertex(glm::vec2 pos, glm::vec2 tex) : pos(pos), tex(tex) {};
        glm::vec2 pos;       
        glm::vec2 tex;
    };
    
    GLuint program;

    struct TerrainRenderable {
        Terrain* terrain { NULL };
        GLuint vb;
        GLuint vao;
        uint32_t num_vertices;
    };

    std::vector<TerrainRenderable> renderables;

    TerrainRenderer() {
        GLuint shaders[2] = { 0 };    
        shaders[0] = gl::CreateShaderFromFile(GL_VERTEX_SHADER, "/Users/eugene.sturm/projects/misc/planet/terrain_vs.glsl");    
        shaders[1] = gl::CreateShaderFromFile(GL_FRAGMENT_SHADER, "/Users/eugene.sturm/projects/misc/planet/terrain_fs.glsl");    
        program = gl::CreateProgram(shaders, 2);

        gpu_tile_buffer = new GPUTileBuffer(RES, 512);
        gpu_tile_cache = new GPUTileCache(gpu_tile_buffer); 
    }
    
    ~TerrainRenderer() {
        delete gpu_tile_cache;
        delete gpu_tile_buffer;
    }

    void GenerateMesh(float size, uint32_t resolution, const glm::vec2& center, std::vector<Vertex>* vertices) {
        float half_size = size / 2.f;
        
        float dx = size / (float)(resolution - 1);
        float dy = size / (float)(resolution - 1);

        auto generate_vertex = [&] (uint32_t i, uint32_t j) -> Vertex { 
            float x = center.x - half_size + (j * dx);
            float y = center.y + half_size - (i * dy);
            float u = j * dx / size;
            float v = i * dy / size;
            
            return Vertex(glm::vec2(x, y), glm::vec2(u, v)); 
        };
    
        for(uint32_t i = 0; i < resolution - 1; ++i) {
            for(uint32_t j = 0; j < resolution - 1; ++j) {
                vertices->push_back(generate_vertex(i, j));
                vertices->push_back(generate_vertex(i+1, j));
                vertices->push_back(generate_vertex(i+1, j+1));

                vertices->push_back(generate_vertex(i+1, j+1));
                vertices->push_back(generate_vertex(i, j+1));
                vertices->push_back(generate_vertex(i, j));
            }
        }
    }  
    

    void RegisterTerrain(Terrain* terrain) {
        std::vector<Vertex> data;
        GenerateMesh(terrain->size, RES, glm::vec2(0, 0), &data);
        TerrainRenderable tr;

        tr.vb = gl::CreateBuffer(GL_ARRAY_BUFFER, data.data(), sizeof(Vertex) * data.size(), GL_STATIC_DRAW);
        GL_CHECK(glGenVertexArrays(1, &tr.vao));
        GL_CHECK(glBindVertexArray(tr.vao));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, tr.vb));
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0));        
        GL_CHECK(glEnableVertexAttribArray(1));
        GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(sizeof(glm::vec2))));        
        tr.num_vertices = data.size(); 
        tr.terrain = terrain;
        renderables.push_back(tr); 
    }


    void Render(glm::mat4 view, glm::mat4 proj, uint32_t viewport_width, float hfov, glm::dvec3 eye) {
//    void Render(const Camera& camera, const Viewport& viewport, const Frustum& frustum) {
        if(renderables.size() == 0) {
           return; 
        }
       
        GL_CHECK(glUseProgram(program));
        GL_CHECK(glActiveTexture(GL_TEXTURE0)); 
        GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, gpu_tile_buffer->tex_id));
        GL_CHECK(glUniform1i(gl::GetUniformLocation(program, "heightmap_tile_array"), 0));
//        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(camera.GetView())));            
//        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(program, "proj"), 1, GL_FALSE, glm::value_ptr(camera.GetProjection())));   
        GL_CHECK(glUniformMatrix4fv(gl::GetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(view)));
        GL_CHECK(glUniformMatrix4fv(gl::GetUniformLocation(program, "proj"), 1, GL_FALSE, glm::value_ptr(proj)));   
        std::queue<TerrainNode*> node_queue;
        
  //      float hfov = camera.GetHorizontalFieldOfView();
  //      glm::dvec3 eye = camera.pos;
  //      uint32_t viewport_width = viewport.width;

        int draw_calls = 0;
        for(const TerrainRenderable& renderable : renderables) {
            GL_CHECK(glBindVertexArray(renderable.vao));
            Terrain* terrain = renderable.terrain; 
            assert(terrain->root);
            node_queue.push(terrain->root);
            while(node_queue.size() > 0) {
                TerrainNode* node = node_queue.front();
                node_queue.pop();
                assert(node);
                double rho = ComputeScreenSpaceError(node->bbox, eye, hfov, viewport_width, node->geometric_error);
//printf("rho:%f\n", rho);
                if(rho < terrain->tau || node->lod_level + 1 > terrain->max_lod) {
                    if(node->children) {
                        // children are not needed, merge?
                    }
                
                    GPUTileCache::Tile* tile = gpu_tile_cache->GetTile(node->lod_level, node->tx, node->ty, [&](GPUTile* tile) -> void {
                        float zmin, zmax; 
                        std::vector<float> heightmap;
                        terrain->heightmap_generator.GenerateRegion(node->world, node->size, &heightmap, &zmin, &zmax);
                        tile->CopyData(RES, RES, heightmap.data());
                        node->bbox.min.z = zmin;
                        node->bbox.max.z = zmax;
                    });

//                    if(frustum.BoxInFrustum(node->bbox)) {                       
                    if(true) {    
     draw_calls++;                   
                        assert(tile);
                        int tile_index = tile->data->index;
                        glm::mat4 world = glm::scale(glm::translate(glm::mat4(), glm::vec3(node->world.x, node->world.y, node->world.z)), glm::vec3(node->size / terrain->size, node->size / terrain->size, node->size / terrain->size));
                        GL_CHECK(glUniformMatrix4fv(gl::GetUniformLocation(program, "world"), 1, GL_FALSE, glm::value_ptr(world)));  
                        GL_CHECK(glUniform1i(gl::GetUniformLocation(program, "array_index"), tile_index));
                        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, renderable.num_vertices));
                        
                    } 
                } else { 
                    if(!node->children) {                                              
                        printf("Splitting LOD:%d %d %d\n", node->lod_level, node->tx, node->ty);
                        double child_size = node->size / 2.0;
                        double half_child_size = child_size / 2.0;
                        glm::dvec3 child_world[4] = {
                            node->world + glm::dvec3(-half_child_size,  half_child_size, 0),
                            node->world + glm::dvec3( half_child_size,  half_child_size, 0),
                            node->world + glm::dvec3(-half_child_size, -half_child_size, 0),
                            node->world + glm::dvec3( half_child_size, -half_child_size, 0)
                        }; 
                        
                        node->children = new TerrainNode[4];
                        for(uint32_t i = 0; i < 4; ++i) {
                            TerrainNode* child = &node->children[i];
                                                      
                            child->world = child_world[i];
                            child->size = child_size;
                            child->lod_level = node->lod_level + 1;
                            child->tx = node->tx * 2 + (i % 2 == 0 ? 0 : 1);
                            child->ty = node->ty * 2 + (i < 2 ? 1 : 0);
                            child->bbox = BoundingBox(
                                    glm::dvec3(child->world.x - half_child_size, child->world.y - half_child_size, 0),
                                    glm::dvec3(child->world.x + half_child_size, child->world.y + half_child_size, 0)
                            );
                            child->geometric_error = node->geometric_error / 2.f;
                        }
                    }

                    for(uint32_t idx = 0; idx < 4; ++idx) {
                        node_queue.push(&node->children[idx]);
                    }   
                }               
            }       
        } 

//            printf("drawcalls2:%d\n", draw_calls);
    }

   
};
    

}


    
#endif
