#ifndef __terrain_renderer2_h__
#define __terrain_renderer2_h__

#include "GLHelpers.h"
#include <vector>
#include <queue>
#include <glm/glm.hpp>
#include "Camera.h"
#include <noise/noise.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp> 
#include "DebugRenderer.h"


struct Vertex {
    Vertex(glm::vec3 pos, glm::vec2 tex) : pos(pos), tex(tex) {};
    Vertex() {};

    glm::vec3 pos;
    glm::vec2 tex;
};

struct BoundingBox {
    BoundingBox(glm::vec3 min, glm::vec3 max) : min(min), max(max) {};
    BoundingBox() {};
    glm::vec3 min;
    glm::vec3 max;
};

double ComputeDistanceFromBoundingBox(const glm::vec3& p, const BoundingBox& bbox) {
    glm::vec3 point_on_bbox = glm::vec3();    
    point_on_bbox.x = (p.x < bbox.min.x) ? bbox.min.x : (p.x > bbox.max.x) ? bbox.max.x : p.x;
    point_on_bbox.y = (p.y < bbox.min.y) ? bbox.min.y : (p.y > bbox.max.y) ? bbox.max.y : p.y;
    point_on_bbox.z = (p.z < bbox.min.z) ? bbox.min.z : ((p.z > bbox.max.z) ? bbox.max.z : p.z);    
    return glm::length(p - point_on_bbox);
}

static int key = 0;




struct ChunkNode {
    int level;
    glm::mat4 world;
    BoundingBox bbox;
    int chunk_id;
    float maximum_geometric_error;
    glm::vec3 center;
    glm::vec2 size;
    glm::uvec2 resolution;
    uint32_t width;
    uint32_t height;
    GLuint vao_id;
    GLuint vb;
    GLuint heightmap_tex;
    uint32_t num_vertices;

    ChunkNode* parent;
    ChunkNode* children;
};

class TerrainRenderer2 {
private:
    ChunkNode* root;
    GLuint vb;
    GLuint vao_id;
    GLuint program;
    GLuint heightmap_tex;
public:
    TerrainRenderer2() {
        root = new ChunkNode();

        GLuint shaders[2] = { 0 };    
        shaders[0] = gl::CreateShaderFromFile(GL_VERTEX_SHADER, "/Users/eugene.sturm/projects/misc/planet/terrain_vs.glsl");    
        shaders[1] = gl::CreateShaderFromFile(GL_FRAGMENT_SHADER, "/Users/eugene.sturm/projects/misc/planet/terrain_fs.glsl");    
        program = gl::CreateProgram(shaders, 2);
    
        glm::vec3 chunk_center = glm::vec3(0, 0, 0);
        glm::vec2 chunk_size = glm::vec2(10000, 10000);
        glm::uvec2 chunk_resolution = glm::uvec2(16, 16);
        std::vector<float> heightmap_data;  
        float max_height = 0;
        float min_height = 0;
        GenerateHeightmap(chunk_center, chunk_size, chunk_resolution, &heightmap_data, &max_height, &min_height); 

        heightmap_tex = gl::CreateTexture2D(GL_R32F, GL_RED, GL_FLOAT, chunk_resolution.x, chunk_resolution.y, heightmap_data.data());        
        std::vector<Vertex> positions;
        GeneratePatch(chunk_center, chunk_size, chunk_resolution, &positions); 

        vb = gl::CreateBuffer(GL_ARRAY_BUFFER, positions.data(), sizeof(Vertex) * positions.size(), GL_STATIC_DRAW);
        GL_CHECK(glGenVertexArrays(1, &vao_id));
        GL_CHECK(glBindVertexArray(vao_id));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vb));
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0));        
        GL_CHECK(glEnableVertexAttribArray(1));
        GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(sizeof(glm::vec3))));        

        root->size = chunk_size;
        root->resolution = chunk_resolution; 
        root->center = chunk_center; 
        root->num_vertices = positions.size();
        root->vao_id = vao_id;
        root->vb = vb;
        root->heightmap_tex = heightmap_tex;
        root->maximum_geometric_error = 32;
        root->chunk_id = ++key;
        root->level = 0;
        root->world = glm::mat4();//glm::rotate(glm::mat4(), -90.f * 3.1415f / 180.f, glm::vec3(1, 0, 0));
        root->bbox = BoundingBox(root->center - glm::vec3(root->size.x / 2.f, root->size.y / 2.f, min_height), 
            root->center + glm::vec3(root->size.x / 2.f, root->size.y / 2.f, min_height));

        DebugRenderer::GetInstance()->DrawBox(root->bbox.min, root->bbox.max);

//        Split(root);
//        Split(&root->children[0]);
    } 

     bool ShouldSplit(const ChunkNode* node, const glm::vec3& eye_pos, float viewport_width, float horizontal_fov_radians) {
        float k = 1.f;        
        double d = ComputeDistanceFromBoundingBox(eye_pos, node->bbox);            
        //printf("d:%f < %f\n", d, k * node->size.x);
        return d < k * node->size.x;
    }

    double ComputeScreenSpaceError(const ChunkNode* node, const glm::vec3& eye_pos, float viewport_width, float horizontal_fov_radians) {                    
        double D = ComputeDistanceFromBoundingBox(eye_pos, node->bbox);            
        double w = 2.f * D * tan(horizontal_fov_radians / 2.f);
        double res = node->maximum_geometric_error * viewport_width / w;        
        return res;        
    }

    void GenerateHeightmap(const glm::vec3& center, const glm::vec2& patch_size, const glm::uvec2& patch_resolution, std::vector<float>* data, float* max_height = NULL, float* min_height = NULL) {

        noise::module::Perlin perlin;
        perlin.SetSeed(0);
        perlin.SetFrequency(0.5);
        perlin.SetOctaveCount(8);
        perlin.SetPersistence(0.25);
        glm::vec2 half_patch_size = patch_size / 2.f;
        float dx = patch_size.x / (float)(patch_resolution.x - 1);
        float dy = patch_size.y / (float)(patch_resolution.y - 1);

        auto sampler = [&center, &patch_size, &half_patch_size, &dx, &dy, &perlin](uint32_t i, uint32_t j) -> float {
            float x = center.x - half_patch_size.x + (j * dx);
            float y = center.y + half_patch_size.y - (i * dy);
           // double val = perlin.GetValue(x * 0.01f, 0, y * 0.01f) * 100.0; 

            double val = 0;
//            printf("sample(i:%d, j:%d) -- x:%f y:%f v:%f\n", i, j, x, y, val);    
//            printf("%d,%d = %f,%f\n", i, j, x, y);

            return val;
        };

        if(max_height != NULL) {
            *max_height = -99999999.f;
        }
        if(min_height != NULL) {
            *min_height = 999999.f;
        }
        for(uint32_t i = 0; i < patch_resolution.y; ++i) {
            for(uint32_t j = 0; j < patch_resolution.x; ++j) {
                double val = sampler(i, j);
                if(max_height != NULL && val > *max_height) {
                    *max_height = val;
                }

                if(min_height != NULL && val < *min_height) {
                    *min_height = val;
                }
                data->push_back(val);
            }
        }
//        printf("\n");

    } 
    void GeneratePatch(const glm::vec3& center, const glm::vec2& patch_size, const glm::uvec2& patch_resolution, std::vector<Vertex>* vertices) {
        glm::vec2 half_patch_size = patch_size / 2.f;
        
        float dx = patch_size.x / (float)(patch_resolution.x - 1);
        float dy = patch_size.y / (float)(patch_resolution.y - 1);

        auto generate_vertex = [&center, &patch_size, &half_patch_size, &dx, &dy] (uint32_t i, uint32_t j) -> Vertex { 
            float x = center.x - half_patch_size.x + (j * dx);
            float y = center.y + half_patch_size.y - (i * dy);
            float u = j * dx / patch_size.x;
            float v = i * dy / patch_size.y;
            
//            printf("vertex(i:%d, j:%d) -- x:%f y:%f ", i, j, x, y);
//            printf("u:%f v:%f\n", u, v);
//            
//            printf("%d,%d = %f,%f\n", i, j, x, y);
            return Vertex(glm::vec3(x, y, 0), glm::vec2(u, v)); 
        };
    
        for(uint32_t i = 0; i < patch_resolution.y - 1; ++i) {
            for(uint32_t j = 0; j < patch_resolution.x - 1; ++j) {
                vertices->push_back(generate_vertex(i, j));
                vertices->push_back(generate_vertex(i+1, j));
                vertices->push_back(generate_vertex(i+1, j+1));

                vertices->push_back(generate_vertex(i+1, j+1));
                vertices->push_back(generate_vertex(i, j+1));
                vertices->push_back(generate_vertex(i, j));
//        printf("\n");
            }
        }
//        printf("Patch c:%f,%f,%f size:%f,%f dx:%f dy:%f\n", center.x, center.y, center.z, patch_size.x, patch_size.y, dx, dy);     
    }  

    
    void Split(ChunkNode* root) {
        if(root->children != NULL) {
//            printf("Node already has children");
            return;            
        }
    
        root->children = new ChunkNode[4];
        glm::vec2 chunk_size = root->size / 2.f;
        glm::vec2 half_chunk_size = chunk_size / 2.f;
        glm::uvec2 chunk_resolution = root->resolution;
        float maximum_geometric_error = root->maximum_geometric_error / 2.f;
              
        glm::vec3 center_positions[] = {
            root->center + glm::vec3(-half_chunk_size.x,  half_chunk_size.y, 0),
            root->center + glm::vec3( half_chunk_size.x,  half_chunk_size.y, 0),            
            root->center + glm::vec3(-half_chunk_size.x, -half_chunk_size.y, 0),
            root->center + glm::vec3( half_chunk_size.x, -half_chunk_size.y, 0),
        };

        std::vector<Vertex> vertices;
        std::vector<float> heightmap; 
        for(uint32_t idx = 0; idx < 4; ++idx) {
            vertices.clear();
            heightmap.clear();

            ChunkNode* node = &root->children[idx];
            float min_height = 0;
            float max_height = 0;
            GenerateHeightmap(center_positions[idx], chunk_size, chunk_resolution, &heightmap, &max_height, &min_height);
            GeneratePatch(center_positions[idx], chunk_size, chunk_resolution, &vertices);
            GLuint vb = gl::CreateBuffer(GL_ARRAY_BUFFER, vertices.data(), sizeof(Vertex) * vertices.size(), GL_STATIC_DRAW);
            GLuint heightmap_id = gl::CreateTexture2D(GL_R32F, GL_RED, GL_FLOAT, chunk_resolution.x, chunk_resolution.y, heightmap.data());
            GLuint vao_id = -1;
            GL_CHECK(glGenVertexArrays(1, &vao_id));
            GL_CHECK(glBindVertexArray(vao_id));
            GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vb));
            GL_CHECK(glEnableVertexAttribArray(0));
            GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0));        
            GL_CHECK(glEnableVertexAttribArray(1));
            GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(sizeof(glm::vec3))));        

            node->vb = vb;
            node->vao_id = vao_id;           
            node->center = center_positions[idx];
            node->resolution = chunk_resolution;
            node->size = chunk_size;
            node->heightmap_tex = heightmap_id;
            node->num_vertices = vertices.size();
            node->parent = root;
            node->children = NULL;
            node->maximum_geometric_error = maximum_geometric_error;
            node->chunk_id = ++key;
            node->world = root->world;
            node->level = root->level + 1;
            node->bbox = BoundingBox(node->center - glm::vec3(node->size.x / 2.f, node->size.y / 2.f, min_height), 
                                node->center + glm::vec3(node->size.x / 2.f, node->size.y / 2.f, min_height));
            //DebugRenderer::GetInstance()->DrawBox(node->bbox.min, node->bbox.max);

          printf("Child ChunkNode: %d h:%f w:%f res:%d,%d center:%f,%f,%f\n", node->level, chunk_size.x, chunk_size.y, chunk_resolution.x, chunk_resolution.y, node->center.x, node->center.y, node->center.z); 
        }
    }
    void Update(glm::vec3 position) {   
        std::queue<ChunkNode*> nodes;
        nodes.push(root);
        while(nodes.size() > 0) {
            ChunkNode* node = nodes.front();
            nodes.pop();
            //ShouldSplit(node, position, 800, 45.f * 3.1415926f / 180.f);
            double rho = ComputeScreenSpaceError(node, position, 800, 45.f * 3.1415926f / 180.f);
            //printf("Computing screen space error between id:%d %f %f %f & %f %f %f := %f\n", node->chunk_id, node->center.x,  node->center.x,  node->center.y, position.x, position.y, position.z, rho);
            if(rho > 5 && node->children == NULL) {
          //  if(ShouldSplit(node, position, 800, 45.f * 3.1415926f / 180.f)) {
                Split(node);
            }
            if(node->children) {
                for(uint32_t idx = 0; idx < 4; ++idx) {                    
                    nodes.push(&node->children[idx]);
                }
            }
        }
       
    }

    void RenderChunkNode(ChunkNode* node) {
        //node->world = glm::rotate(node->world, -0.01f, glm::vec3(1, 0, 0));
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(program, "world"), 1, GL_FALSE, glm::value_ptr(node->world)));                        
        GL_CHECK(glBindVertexArray(node->vao_id));

        GL_CHECK(glActiveTexture(GL_TEXTURE0));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, node->heightmap_tex));
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, node->num_vertices));          
    }
    
     
    void Render(Camera& cam) {
        GL_CHECK(glUseProgram(program));
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(program, "view_proj"), 1, GL_FALSE, glm::value_ptr(cam.GetProjection() * cam.GetView())));                        
        GL_CHECK(glUniform1i(glGetUniformLocation(program, "heightmap"), 0));

        std::queue<ChunkNode*> nodes;
        nodes.push(root);
        int draw_calls = 0;

        while(nodes.size() > 0) {
            ChunkNode* node = nodes.front();
            nodes.pop();

            if(node->children) {
                
                double rho = ComputeScreenSpaceError(node, cam.GetPos(), 800, 45.f * 3.1415926f / 180.f);
                if(rho < 5) {
                //if(!ShouldSplit(node, cam.GetPos(), 800, 45.f * 3.1415926f / 180.f)) {
                    RenderChunkNode(node);draw_calls++;
                } else {
                    for(uint32_t idx = 0; idx < 4; ++idx) {
                        nodes.push(&node->children[idx]);
                    }
                }
            } else {
                RenderChunkNode(node);
                draw_calls++;
            }
        }

        //printf("drawcalls1:%d\n", draw_calls);


//        RenderChunkNode(root);
//        RenderChunkNode(&root->children[3]);
//        RenderChunkNode(&root->children[1]);
//        RenderChunkNode(&root->children[2]);
//        RenderChunkNode(&root->children[0].children[0]);
//        RenderChunkNode(&root->children[0].children[1]);
//        RenderChunkNode(&root->children[0].children[2]);
//        RenderChunkNode(&root->children[0].children[3]);
    }

};


#endif

