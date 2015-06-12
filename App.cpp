#include "App.h"
#include "GLHelpers.h"
#include <glm/glm.hpp>
#include "Camera.h"
#include <vector>
#include "System.h"
#include <sstream>
#include "Helpers.h"
#include <list>
#include <unordered_map>
#include "BoundingBox.h"
#include <queue>
#include <glm/gtx/transform.hpp>
#include <noise/noise.h>
#include "Frustum.h"
#include "DebugRenderer.h"
#include <glm/gtc/quaternion.hpp> 
#include <glm/gtx/quaternion.hpp>

uint32_t frame_count = 0;
double curr_frame_time = 0;
double prev_frame_time = 0;
double accumulate = 0;
double total_frame_count = 0;
double frame_time = 0;
Camera cam;
float mouse_speed = 1.f;
float walk_speed = 300.f;
GLuint program;
GLuint vao;
GLuint vb;
uint32_t num_vertices = 0;



struct GPUTile {
    GPUTile() {};
    GPUTile(GLuint tex_id, uint32_t idx) : texture_array_id(tex_id), index(idx) {};

    GLuint texture_array_id;
    uint32_t index;

    void CopyData(uint32_t width, uint32_t height, void* data) { 
        gl::UpdateTexture2DArray(texture_array_id, GL_RED, GL_FLOAT, width, height, index, data);    
    }
};

class GPUTileBuffer {
private:
    uint32_t tile_size;
    uint32_t num_tiles;
    GLuint tex_id;    
    GPUTile* tiles;
    std::list<GPUTile*> unused;
    std::list<GPUTile*> used;
public:
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

    ~GPUTileBuffer() {        
        delete [] tiles;
    }

    GPUTile* GetFreeTile() {        
        GPUTile* tile = unused.front();        
        if(tile) {
            unused.pop_front();
            used.push_back(tile);
            LOG_D("GPUTileBuffer: used:%d unused:%d", used.size(), unused.size()); 
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

    GLuint GetTextureId() {
        return tex_id;
    }
};


struct Tile {
    Tile(uint32_t lod, uint32_t tx, uint32_t ty, GPUTile* data) : lod(lod), tx(tx), ty(ty), data(data) {};
    
    uint32_t lod;
    uint32_t tx;
    uint32_t ty;

    GPUTile* data;
};

class LRUTileCache {
private:
    typedef size_t TileId;
    typedef typename std::unordered_map<TileId, Tile*>::iterator CacheIterator;

    GPUTileBuffer* gpu_tile_buffer;
    std::list<TileId> lru;
    std::unordered_map<TileId, Tile*> cache;        

private:
     TileId GetTileId(uint32_t lod, uint32_t tx, uint32_t ty) {
        size_t seed = 0;
        HashCombine(seed, lod);
        HashCombine(seed, tx);
        HashCombine(seed, ty);
        return seed;
    } 
public:
    LRUTileCache(GPUTileBuffer* gpu_tile_buffer) : gpu_tile_buffer(gpu_tile_buffer) {}
    ~LRUTileCache() {
        for(std::pair<TileId, Tile*> p : cache) {
            delete p.second;
        }        
    }
    Tile* Get(uint32_t lod, uint32_t tx, uint32_t ty, std::function<void(GPUTile* gpu_tile)> load_func) {
        TileId key = GetTileId(lod, tx, ty);
        CacheIterator it = cache.find(key);

        Tile* val = NULL;        
        if(it != cache.end()) {            
            lru.remove(key);
            lru.push_front(key);
            val = it->second;
        } else {            
            GPUTile* gpu_tile = gpu_tile_buffer->GetFreeTile();
            if(gpu_tile) {
                load_func(gpu_tile);                
                val = new Tile(lod, tx, ty, gpu_tile);
                lru.push_front(key);
                cache.insert(std::make_pair(key, val));                                
            } else {           
                TileId key_to_evict = lru.back();
                it = cache.find(key_to_evict);
                if(it != cache.end()) {                    
                    lru.pop_back();
                    cache.erase(it);

                    val = it->second;
                    //LOG_D("Evicting %d %d %d for %d %d %d", val->lod, val->tx, val->ty, lod, tx, ty); 
                    val->lod = lod;
                    val->tx = tx;
                    val->ty = ty;
                    load_func(val->data);
                                            
                    lru.push_front(key);
                    cache.insert(std::make_pair(key, val));                    
                } else {
                    // boom
                    LOG_E("%s", "Failed to evict from LRUTileCache");
                    assert(false);
                }
            }
        }
        return val;
    }        
};



template <class T>
void BuildGrid(float center_x, float center_y, float center_z, 
    float size_x, float size_y, 
    uint32_t resolution_x, uint32_t resolution_y, 
    std::function<T(float x, float y, float u, float v)> vertex_generator, 
    std::vector<T>* vertices) 
{
    float half_size_x = size_x / 2.f;
    float half_size_y = size_y / 2.f;
    
    float dx = size_x / (float)(resolution_x - 1);
    float dy = size_y / (float)(resolution_y - 1);

    auto generate_vertex = [&] (uint32_t i, uint32_t j) -> T { 
        float x = center_x - half_size_x + (j * dx);
        float y = center_y + half_size_y - (i * dy);
        float u = j * dx / size_x;
        float v = i * dy / size_y;
        
        return vertex_generator(x, y, u, v); 
    };

    for(uint32_t i = 0; i < resolution_y - 1; ++i) {
        for(uint32_t j = 0; j < resolution_x - 1; ++j) {
            vertices->push_back(generate_vertex(i, j));
            vertices->push_back(generate_vertex(i+1, j));
            vertices->push_back(generate_vertex(i+1, j+1));

            vertices->push_back(generate_vertex(i+1, j+1));
            vertices->push_back(generate_vertex(i, j+1));
            vertices->push_back(generate_vertex(i, j));
        }
    }
}  

template <class T>
void BuildGrid(const glm::vec3& center, const glm::vec2& size, const glm::uvec2& resolution,     
    std::function<T(float x, float y, float u, float v)> vertex_generator, 
    std::vector<T>* vertices) 
{
    BuildGrid(center.x, center.y, center.z, size.x, size.y, resolution.x, resolution.y,
        vertex_generator, vertices);
}

template <class T>
void GenerateHeightmapRegion(const glm::vec3& center, const glm::vec2& size, const glm::uvec2& resolution, 
    std::function<T(double x, double y, double z)> heightmap_generator, 
    std::vector<T>* data, T* max, T* min) {

    *max = -99999999;
    *min = 9999999;
    glm::vec2 half_size = size / 2.f;
    float dx = size.x / (float)(resolution.x - 1);
    float dy = size.y / (float)(resolution.y - 1);
    for(uint32_t i = 0; i < resolution.y; ++i) {
        for(uint32_t j = 0; j < resolution.x; ++j) {
            float x = center.x - half_size.x + (j * dx);
            float y = center.y + half_size.y - (i * dy);
            float z = 0;
            T val = heightmap_generator(x, y, z);
            if(val > *max) *max = val;
            else if(val < *min) *min = val;
            data->push_back(val);
        }
    }
}

struct TerrainNode {
    uint32_t lod;
    uint32_t tx;
    uint32_t ty;

    double x;
    double y;

    BoundingBox bbox;
    float size;

    TerrainNode* children = { NULL };
};


uint32_t max_lod = 20;


double ComputeScreenSpaceError2(const BoundingBox& bbox, const glm::vec3& eye, float size) {
    double d = bbox.GetDistanceFromBoundingBox(eye);            
    if(d < 1.f * size) {
        return 100;
    } else {
        return 0;
    }
}
double ComputeScreenSpaceError(const BoundingBox& bbox, const glm::vec3& eye, float hfov, uint32_t viewport_width, float geometric_error) {
    double d = bbox.GetDistanceFromBoundingBox(eye);            

    double w = 2.f * d * tan(hfov / 2.f);
    if(w == 0) {
        return 0; //TODO: this mean if we are ever inside the bounding box, we say we have 0 screen space error. NOT good.
    }
    double res = geometric_error * viewport_width / w;        
    return res;   
}

GPUTileBuffer* gpu_tile_buffer;
LRUTileCache* lru_tile_cache;
TerrainNode* root;
std::function<void(GPUTile* tile)> generate_heightmap_func;
const uint32_t RESOLUTION = 64;

void HandleInput(const app::KeyState& key_state, const app::CursorState& cursor_state, float dt) {

    glm::vec3 translation(0, 0, 0);
    if (key_state.IsPressed(app::KeyCode::KEY_1)) {
        walk_speed = 600.f;
    }

    if (key_state.IsPressed(app::KeyCode::KEY_2)) {
        walk_speed = 300.f;
    }

    if (key_state.IsPressed(app::KeyCode::KEY_3)) {
        walk_speed = 15.f;
    }

    if (key_state.IsPressed(app::KeyCode::KEY_W)) {
        translation.z += walk_speed * dt;
    }

    if (key_state.IsPressed(app::KeyCode::KEY_A)) {
        translation.x -= walk_speed * dt;
    }

    if (key_state.IsPressed(app::KeyCode::KEY_S)) {
        translation.z -= walk_speed * dt;
    }

    if (key_state.IsPressed(app::KeyCode::KEY_D)) {
        translation.x += walk_speed * dt;
    }

    if (key_state.IsPressed(app::KeyCode::KEY_Q)) {
        translation.y += walk_speed * dt;
    }

    if (key_state.IsPressed(app::KeyCode::KEY_E)) {
        translation.y -= walk_speed * dt;
    }

    if (key_state.IsPressed(app::KeyCode::KEY_W)) {
        translation.z += walk_speed * dt;
    }

    cam.Translate(translation);
    if (translation != glm::vec3(0, 0, 0)) {

    }

    glm::vec2 mouse_delta(cursor_state.delta_x, cursor_state.delta_y);
    if(mouse_delta.x != 0 || mouse_delta.y != 0) {
        float pitch = mouse_speed * dt * mouse_delta.y;
        float yaw = mouse_speed * dt * mouse_delta.x;
        cam.Pitch(pitch);
        cam.Yaw(-yaw);
    }
}

struct Vertex { 
    Vertex(glm::vec2 pos, glm::vec2 tex) : pos(pos), tex(tex) {};
    glm::vec2 pos;
    glm::vec2 tex;
};

struct Transform {    
    glm::quat rotation;
    glm::vec3 position;    

    void ConvertEulerAngles(float x_radians, float y_radians, float z_radians) {
        rotation = glm::quat(glm::vec3(x_radians, y_radians, z_radians));
    }
};

struct Terrain {
    uint32_t max_lod;
    glm::mat4 world;
    uint32_t size;
    uint32_t resolution;
    float split_factor;

    TerrainNode* root { NULL };
};

Terrain terrain;

void App::OnStart() {    
    LOG_D("GL_VERSION: %s", glGetString(GL_VERSION));
    LOG_D("GL_SHADING_LANGUAGE_VERSION: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    LOG_D("GL_VENDOR: %s", glGetString(GL_VENDOR));
    LOG_D("GL_RENDERER: %s", glGetString(GL_RENDERER));
    glClearColor(0.1f, 0.1f, 0.1f, 1.f);    
    //glEnable (GL_DEPTH_TEST);    
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    cam.MoveTo(0, 0, 1000);
    cam.LookAt(0, 0, 0);   

    GLuint shaders[2] = { 0 };    
    shaders[0] = gl::CreateShaderFromFile(GL_VERTEX_SHADER, "/Users/eugene.sturm/projects/misc/planet72/terrain_vs.glsl");    
    shaders[1] = gl::CreateShaderFromFile(GL_FRAGMENT_SHADER, "/Users/eugene.sturm/projects/misc/planet72/terrain_fs.glsl");    
    program = gl::CreateProgram(shaders, 2);

    std::vector<Vertex> vertices;
    auto vertex_generator = [&](float x, float y, float u, float v) -> Vertex  {     
        //LOG_D("%f %f : %f %f", x, y, u, v);         
        return Vertex(glm::vec2(x, y), glm::vec2(u, v));
    };

    float best = 0.1;
    
    uint32_t size = 10000000;
    //max_lod = (int)((log(size/res/best) / log(2)) + 0.5);
    //LOG_D("MAX_LOD: %d %f", max_lod, pow(2, max_lod));
    max_lod = 16;

    terrain.resolution = 64;
    terrain.size = size;
    terrain.split_factor = 1.f;
    terrain.max_lod = max_lod;
    terrain.world = glm::mat4();    

    root = new TerrainNode();
    root->lod = 0;
    root->tx = 0;
    root->ty = 0;
    root->x = 0;
    root->y = 0;
    root->size = terrain.size;
    root->bbox.max = glm::vec3(root->size / 2.f, root->size / 2.f, 0);
    root->bbox.min = glm::vec3(-root->size / 2.f, -root->size / 2.f, 0);

    terrain.root = root;

    BuildGrid<Vertex>(glm::vec3(0, 0, 0), glm::vec2(size, size), glm::uvec2(RESOLUTION, RESOLUTION), vertex_generator, &vertices);

    num_vertices = vertices.size();    
    vb = gl::CreateBuffer(GL_ARRAY_BUFFER, vertices.data(), sizeof(Vertex) * vertices.size(), GL_STATIC_DRAW);
    vao = gl::CreateVertexArrayObject(vb, { { ParamType::Float2, ParamType::Float2 } });
    gpu_tile_buffer = new GPUTileBuffer(RESOLUTION, 512);
    lru_tile_cache = new LRUTileCache(gpu_tile_buffer);
    

 


    
}

void App::OnFrame(const app::AppState* app_state, float dt) {    
    HandleInput(app_state->key_state, app_state->cursor_state, 1.f/60.f);

    glm::mat4 proj = cam.BuildProjection();
    glm::mat4 view = cam.BuildView();
    glm::mat4 world = glm::mat4();
    Frustum frustum(proj, view);

    glClear(GL_COLOR_BUFFER_BIT);

    //DebugRenderer::GetInstance()->Render(proj * view);

    GL_CHECK(glBindVertexArray(vao));
    GL_CHECK(glUseProgram(program));
    
    int tex_slot = 0;

    gl::SetUniform(program, "proj", ParamType::Float4x4, &proj);
    gl::SetUniform(program, "view", ParamType::Float4x4, &view);        
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, gpu_tile_buffer->GetTextureId()));    
    gl::SetUniform(program, "heightmap_tile_array", ParamType::Int32, &tex_slot);    

    
    std::queue<TerrainNode*> q;
    q.push(root);

    while(!q.empty()) {
        TerrainNode* node = q.front();
        q.pop();        
        //double rho = ComputeScreenSpaceError(node->bbox, cam.pos, ToRadians(cam.fov_degrees), 800, node->geometric_error);        
        double rho = ComputeScreenSpaceError2(node->bbox, cam.pos, node->size);        
        if(rho < 5 || node->lod >= max_lod) {
            generate_heightmap_func = [&](GPUTile* tile) -> void {
                std::vector<float> data;
                float min, max;

                GenerateHeightmapRegion<float>(glm::vec3(node->x, node->y, 0), glm::vec2(node->size, node->size), 
                    glm::uvec2(RESOLUTION, RESOLUTION), [&](double x, double y, double z) -> float {                        
                        noise::module::Perlin perlin;
                        perlin.SetSeed(0);
                        perlin.SetFrequency(0.5);
                        perlin.SetOctaveCount(8);
                        perlin.SetPersistence(0.25); 
                        return perlin.GetValue(x * 0.1f, y * 0.1f, z * 0.1f) * 20;                                                                                     
                    }, &data, &max, &min);

                node->bbox.min.z = min;
                node->bbox.max.z = max;
                //DebugRenderer::GetInstance()->DrawBox(node->bbox.min, node->bbox.max);
                tile->CopyData(RESOLUTION, RESOLUTION, data.data());                
            }; 
            
            Tile* tile = lru_tile_cache->Get(node->lod, node->tx, node->ty, generate_heightmap_func);            

/*            if(!frustum.IsBoxInFrustum(node->bbox)) {
                continue;
            }
*/
            float scale_factor = node->size / (node->size * pow(2, (node->lod)));            
            glm::mat4 translation = glm::translate(glm::mat4(), glm::vec3(node->x, node->y, 0));
            glm::mat4 scale = glm::scale(glm::vec3(scale_factor, scale_factor, 1.f));
            world = translation * scale;
            int tile_index = tile->data->index;
            gl::SetUniform(program, "tile_index", ParamType::Int32, &tile_index);    
            gl::SetUniform(program, "world", ParamType::Float4x4, &world);
            GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, num_vertices));
        } else {
            if(!node->children) {                
                float child_size = node->size / 2.0;
                float half_child_size = child_size / 2.0;
                double child_pos[4][2] = {
                    { node->x - half_child_size, node->y + half_child_size },
                    { node->x + half_child_size, node->y + half_child_size },
                    { node->x - half_child_size, node->y - half_child_size },
                    { node->x + half_child_size, node->y - half_child_size },
                };               

                node->children = new TerrainNode[4];
                for(uint32_t i = 0; i < 4; ++i) {
                    TerrainNode* child = &node->children[i];
                                              
                    child->x = child_pos[i][0];
                    child->y = child_pos[i][1];
                    child->size = child_size;
                    child->lod = node->lod + 1;
                    child->tx = node->tx * 2 + (i % 2 == 0 ? 0 : 1);
                    child->ty = node->ty * 2 + (i < 2 ? 1 : 0);
                    child->bbox.max = glm::vec3(child->x + half_child_size, child->y + half_child_size, 0);
                    child->bbox.min = glm::vec3(child->x - half_child_size, child->y - half_child_size, 0);  
                }
            }

            for(uint32_t idx = 0; idx < 4; ++idx) {
                q.push(&node->children[idx]);
            }   
        }
    }    

    
    

    accumulate += dt;
    ++frame_count;
    ++total_frame_count;    

    if(accumulate > 1.0) {
        std::stringstream ss;
        ss << "gfx | FPS: " << frame_count << " | Frame: " << total_frame_count;
        ss << " | Pos: " << cam.pos;
        sys::SetWindowTitle(ss.str().c_str());
        frame_count = 0;
        accumulate = 0.0;
    }
}
void App::OnShutdown() {

}
