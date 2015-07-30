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
#include <algorithm>
#include "TerrainRenderer.h"
#include "Log.h"

#ifdef _WIN32
#include <functional>
#endif

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
uint32_t max_lod;
float split_factor;
GPUTileBuffer* heightmap_normals_buffer;
LRUTileCache* heightmap_normals_cache;
GPUTileBuffer* gpu_tile_buffer;
LRUTileCache* lru_tile_cache;
TerrainNode* root;
std::function<void(GPUTile* tile)> generate_heightmap_func;
const uint32_t TERRAIN_RESOLUTION = 32;
const uint32_t TERRAIN_SIZE = 10000;
const float TERRAIN_SPLIT_FACTOR = 1.0f;
const uint32_t TERRAIN_MAX_LOD = 5;
Terrain terrain;
std::vector<Terrain> terrains;

template <class T>
void BuildVertexGrid(
    float center_x, float center_y, float center_z, 
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
void BuildVertexGrid(const glm::vec3& center, const glm::vec2& size, const glm::uvec2& resolution,     
    std::function<T(float x, float y, float u, float v)> vertex_generator, 
    std::vector<T>* vertices) 
{
    BuildVertexGrid(center.x, center.y, center.z, size.x, size.y, resolution.x, resolution.y,
        vertex_generator, vertices);
}

template <class T>
void GenerateHeightmapRegion(const glm::vec3& center, 
    const glm::vec2& size, 
    const glm::uvec2& resolution, 
    std::function<T(double x, double y, double z)> heightmap_generator, 
    std::vector<T>* data, T* max, T* min) {

    *max = std::numeric_limits<float>::min();
    *min = std::numeric_limits<float>::max();
    glm::vec2 half_size = size / 2.f;
    float dx =  .x / (float)(resolution.x - 1);
    float dy = size.y / (float)(resolution.y - 1);
    for(uint32_t i = 0; i < resolution.y; ++i) {
        for(uint32_t j = 0; j < resolution.x; ++j) {
            float x = center.x - half_size.x + (j * dx);
            float y = center.y + half_size.y - (i * dy);
            float z = 0;

            T val = heightmap_generator(x, y, z);

            if(val > *max) {
                *max = val;      
            } else if(val < *min) {
                *min = val;
            }

            data->push_back(val);
        }
    }
}

void GenerateHeightmapRegionNormals(const std::vector<float>& heightmap_data, 
    const glm::vec2& size, 
    const glm::uvec2& resolution, 
    uint32_t lod,
    std::vector<glm::vec3>* generated_normal_data) {
    
    // convert from 2D index to 1D index, clamping to edges (ex. i=-1 => i=0)
    auto get_index = [&](int32_t i, int32_t j) -> int32_t {
        int32_t k = (i >= (int32_t)resolution.x) ? resolution.x - 1 : ((i < 0) ? 0 : i);
        int32_t p = (j >= (int32_t)resolution.y) ? resolution.y - 1 : ((j < 0) ? 0 : j);
       // LOG_D("%d %d %d -> %d %d", j >= resolution.y, i, j, k, p);
        return k * resolution.y + p;
    };
    
    // Note(eugene): need to do this on gpu
    for(uint32_t i = 0; i < resolution.x; ++i) {
        for(uint32_t j = 0; j < resolution.y; ++j) {
            float tl = heightmap_data[get_index(i - 1, j - 1)];
            float t  = heightmap_data[get_index(i - 1, j)];
            float tr = heightmap_data[get_index(i - 1, j + 1)];
            float r  = heightmap_data[get_index(i, j + 1)];
            float br = heightmap_data[get_index(i + 1, j + 1)];
            float b  = heightmap_data[get_index(i + 1, j)];
            float bl = heightmap_data[get_index(i + 1, j - 1)];
            float l  = heightmap_data[get_index(i, j - 1)];


            float f = resolution.x / 128.f;
            float x = -((br - bl) + (2.f * (r - l)) + (tr-tl));
            float y = -((tl - bl) + (2.f * (t - b)) + (tr-br));
            float z = 1.f / pow(2, (lod)); 
            glm::vec3 normal = glm::normalize(glm::vec3(f*x, f*y, z));
            
        
            generated_normal_data->push_back(normal);
            //LOG_D("%f %s", glm::dot(glm::vec3(0, 0, 1), normal), ToString(normal).c_str());
            //LOG_D("%d %d(%d) %.5f -- %d:%.5f %d:%.5f %d:%.5f %d:%.5f %s", i, j, get_index(i, j), heightmap_data[get_index(i, j)], get_index(i + 1, j), y_u, get_index(i - 1, j), y_d, get_index(i, j-1), x_l, get_index(i, j+1), x_r, ToString(glm::normalize(glm::vec3(x, y, 1))).c_str());
        }
    }

}

void GenerateHeightmapRegion(uint32_t lod, const glm::vec3& center, const glm::vec2& size, const glm::uvec2& resolution,
                             std::function<float(double x, double y, double z)> heightmap_generator,
                             std::vector<float>* elevation_data, float* elevation_max, float* elevation_min,
                             std::vector<glm::vec3>* normal_data) {    
    GenerateHeightmapRegion(center, size, resolution, heightmap_generator, elevation_data, elevation_max, elevation_min);
    GenerateHeightmapRegionNormals(*elevation_data, size, resolution, lod, normal_data);
}


double ComputeScreenSpaceError2(const BoundingBox& bbox, const glm::vec3& eye, float size) {
    double d = bbox.GetDistanceFromBoundingBox(eye);            
    if(d < split_factor * size) {
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


void HandleInput(const app::KeyState& key_state, const app::CursorState& cursor_state, float dt) {

    glm::vec3 translation(0, 0, 0);
    if (key_state.IsPressed(app::KeyCode::KEY_1)) {
        walk_speed = 600.f;
    }

    if (key_state.IsPressed(app::KeyCode::KEY_2)) {
        walk_speed = 100.f;
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
};

  auto vertex_generator = [&](float x, float y, float u, float v) -> Vertex  {             
        return Vertex(glm::vec2(x, y), glm::vec2(u, v));
    };


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
//    glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 //   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    cam.MoveTo(0, 0, 1000);
    cam.LookAt(0, 0, 0);   

    GLuint shaders[2] = { 0 };
	shaders[0] = gl::CreateShaderFromFile(GL_VERTEX_SHADER, "/Users/eugene.sturm/projects/misc/planet72/shaders/terrain_vs.glsl");
	shaders[1] = gl::CreateShaderFromFile(GL_FRAGMENT_SHADER, "/Users/eugene.sturm/projects/misc/planet72/shaders/terrain_fs.glsl");
    program = gl::CreateProgram(shaders, 2);

    std::vector<Vertex> vertices;

    float best = 0.1;
    
    uint32_t size = TERRAIN_SIZE;
    //max_lod = (int)((log(size/res/best) / log(2)) + 0.5);
    //LOG_D("MAX_LOD: %d %f", max_lod, pow(2, max_lod));
    max_lod = TERRAIN_MAX_LOD;
    split_factor = TERRAIN_SPLIT_FACTOR;

    terrain.resolution = 128;
    terrain.size = size;
    terrain.split_factor = 1.f;
    terrain.max_lod = max_lod;
    //terrain.transform.position = glm::vec3(0, 0, 0);    
    
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

    BuildVertexGrid<Vertex>(glm::vec3(0, 0, 1), glm::vec2(size, size), glm::uvec2(TERRAIN_RESOLUTION, TERRAIN_RESOLUTION), vertex_generator, &vertices);

    num_vertices = vertices.size();    
    vb = gl::CreateBuffer(GL_ARRAY_BUFFER, vertices.data(), sizeof(Vertex) * vertices.size(), GL_STATIC_DRAW);
    vao = gl::CreateVertexArrayObject(vb, { { ParamType::Float2, ParamType::Float2 } });
    gpu_tile_buffer = new GPUTileBuffer(TERRAIN_RESOLUTION, 512, GL_R32F);
    lru_tile_cache = new LRUTileCache(gpu_tile_buffer);
    heightmap_normals_buffer = new GPUTileBuffer(TERRAIN_RESOLUTION, 512, GL_RGB32F);
    heightmap_normals_cache = new LRUTileCache(heightmap_normals_buffer);
    root->vao = vao;
    root->vb = vb;
}


void App::OnFrame(const app::AppState* app_state, float dt) {
    HandleInput(app_state->key_state, app_state->cursor_state, 1.f/60.f);

    glm::mat4 proj = cam.BuildProjection();
    glm::mat4 view = cam.BuildView();
    glm::mat4 world = glm::mat4();
    Frustum frustum(proj, view);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    //DebugRenderer::GetInstance()->Render(proj * view);

//    GL_CHECK(glBindVertexArray(vao));
    GL_CHECK(glUseProgram(program));
    
    int tex_slot = 0;

    gl::SetUniform(program, "proj", ParamType::Float4x4, &proj);
    gl::SetUniform(program, "view", ParamType::Float4x4, &view);        
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, gpu_tile_buffer->GetTextureId()));    
    gl::SetUniform(program, "heightmap_elevations_tile_array", ParamType::Int32, &tex_slot);
    
    tex_slot = 1;
    GL_CHECK(glActiveTexture(GL_TEXTURE1));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, heightmap_normals_buffer->GetTextureId()));
    gl::SetUniform(program, "heightmap_normals_tile_array", ParamType::Int32, &tex_slot);
    
    std::queue<TerrainNode*> q;
    q.push(root);

    while(!q.empty()) {
        TerrainNode* node = q.front();
        q.pop();        
        //double rho = ComputeScreenSpaceError(node->bbox, cam.pos, ToRadians(cam.fov_degrees), 800, node->geometric_error);        
        double rho = ComputeScreenSpaceError2(node->bbox, cam.pos, node->size);        
        if(rho < 5 || node->lod >= max_lod) {
            std::vector<float> elevation_data;
            std::vector<glm::vec3> normal_data;
            
            generate_heightmap_func = [&](GPUTile* tile) -> void {

                float min, max;
                glm::uvec2 resolution = glm::uvec2(TERRAIN_RESOLUTION, TERRAIN_RESOLUTION);
                GenerateHeightmapRegion(node->lod, glm::vec3(node->x, node->y, 0), glm::vec2(node->size, node->size),
                    resolution, [&](double x, double y, double z) -> float {
                        noise::module::RidgedMulti mountain;
                        mountain.SetSeed(32);
                        mountain.SetFrequency(0.05);
                        mountain.SetOctaveCount(8);
  //                      perlin.SetPersistence(0.25);
double v = mountain.GetValue(x * 0.01f, y * 0.01f, z * 0.01f);
                        return v;
//                        return sin(x * 3.14f/180.f);
//                        return x < 0 &&  y < 0 ? 1.f : -1.f;
                    }, &elevation_data, &max, &min, &normal_data);
                

                node->bbox.min.z = min;
                node->bbox.max.z = max;
                //DebugRenderer::GetInstance()->DrawBox(node->bbox.min, node->bbox.max);
                tile->CopyData(TERRAIN_RESOLUTION, TERRAIN_RESOLUTION, GL_RED, elevation_data.data());
                
            };
            

            std::function<void(GPUTile* tile)> pass_normals_func = [&](GPUTile* tile) -> void {
                tile->CopyData(TERRAIN_RESOLUTION, TERRAIN_RESOLUTION, GL_RGB, normal_data.data());
            };

            

            Tile* elevations_tile = lru_tile_cache->Get(node->lod, node->tx, node->ty, generate_heightmap_func);
            Tile* normals_tile = heightmap_normals_cache->Get(node->lod, node->tx, node->ty, pass_normals_func);

/*
            if(!frustum.IsBoxInFrustum(node->bbox)) {
                continue;
            }
*/
            float scale_factor = node->size / (node->size * pow(2, (node->lod)));
//            printf("%f\n", scale_factor);
            glm::mat4 translation = glm::translate(glm::mat4(), glm::vec3(node->x, node->y, 0));            
            glm::mat4 scale = glm::scale(glm::vec3(scale_factor, scale_factor, 1.f));
            world = glm::mat4();//translation * scale;
            int elevations_tile_index = elevations_tile->data->index;
            int normals_tile_index = normals_tile->data->index;
            gl::SetUniform(program, "elevations_tile_index", ParamType::Int32, &elevations_tile_index);
            gl::SetUniform(program, "normals_tile_index", ParamType::Int32, &normals_tile_index);
            gl::SetUniform(program, "world", ParamType::Float4x4, &world);
            
            GL_CHECK(glBindVertexArray(node->vao));
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


                    // trying out a mesh for each quad...not the end game solution
                    std::vector<Vertex> vertices;
                    BuildVertexGrid<Vertex>(glm::vec3(child->x, child->y, 1), glm::vec2(child->size, child->size), glm::uvec2(TERRAIN_RESOLUTION, TERRAIN_RESOLUTION), vertex_generator, &vertices);
                    num_vertices = vertices.size();    
                    child->vb = gl::CreateBuffer(GL_ARRAY_BUFFER, vertices.data(), sizeof(Vertex) * vertices.size(), GL_STATIC_DRAW);
                    child->vao = gl::CreateVertexArrayObject(child->vb, { { ParamType::Float2, ParamType::Float2 } });
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
