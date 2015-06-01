#ifndef __terrain_renderer2_h__
#define __terrain_renderer2_h__

#include "GLHelpers.h"
#include <vector>
#include <queue>
#include <glm/glm.hpp>
#include "Camera.h"
#include <noise/noise.h>
#include <glm/gtc/type_ptr.hpp>


struct Vertex {
    Vertex(glm::vec3 pos, glm::vec2 tex) : pos(pos), tex(tex) {};
    Vertex() {};

    glm::vec3 pos;
    glm::vec2 tex;
};

struct ChunkNode {
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
        glm::vec2 chunk_size = glm::vec2(1000, 1000);
        glm::uvec2 chunk_resolution = glm::uvec2(4, 4);
        std::vector<float> heightmap_data;  
       
        GenerateHeightmap(chunk_center, chunk_size, chunk_resolution, &heightmap_data); 

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

        Split(root);
        Split(&root->children[0]);
    } 

    double ComputeScreenSpaceError(const ChunkNode* node, const glm::vec3& eye_position) {
        if(glm::distance(node->center, eye_position) > 10) {
            return 0;
        } else {
            return 1;
        }
    }

    void GenerateHeightmap(const glm::vec3& center, const glm::vec2& patch_size, const glm::uvec2& patch_resolution, std::vector<float>* data) {

        noise::module::Perlin perlin;
        perlin.SetSeed(0);
        glm::vec2 half_patch_size = patch_size / 2.f;
        float dx = patch_size.x / (float)(patch_resolution.x - 1);
        float dy = patch_size.y / (float)(patch_resolution.y - 1);

        auto sampler = [&center, &patch_size, &half_patch_size, &dx, &dy, &perlin](uint32_t i, uint32_t j) -> float {
            float x = center.x - half_patch_size.x + (j * dx);
            float y = center.y + half_patch_size.y - (i * dy);
            double val = perlin.GetValue(x * 0.01f, 0, y * 0.01f) * 30.0; 
//            double val = /*cos(x * 3.14159f / 180.f) **/ sin(y * 3.14159f/180.f) * 50;
//            double val = y >= 0 ? 50 : 0;          
            printf("sample(i:%d, j:%d) -- x:%f y:%f v:%f\n", i, j, x, y, val);    
//            printf("%d,%d = %f,%f\n", i, j, x, y);

            return val;
        };

        for(uint32_t i = 0; i < patch_resolution.y; ++i) {
            for(uint32_t j = 0; j < patch_resolution.x; ++j) {
                data->push_back(sampler(i, j));
            }
        }
        printf("\n");

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
            
            printf("vertex(i:%d, j:%d) -- x:%f y:%f ", i, j, x, y);
            printf("u:%f v:%f\n", u, v);
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
        printf("\n");
            }
        }
        printf("Patch c:%f,%f,%f size:%f,%f dx:%f dy:%f\n", center.x, center.y, center.z, patch_size.x, patch_size.y, dx, dy);     
    }  

    
    void Split(ChunkNode* root) {
        if(root->children != NULL) {
            printf("Node already has children");
            return;            
        }
    
        root->children = new ChunkNode[4];
        glm::vec2 chunk_size = root->size / 2.f;
        glm::vec2 half_chunk_size = chunk_size / 2.f;
        glm::uvec2 chunk_resolution = root->resolution;

              
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
            GenerateHeightmap(center_positions[idx], chunk_size, chunk_resolution, &heightmap);
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

          printf("Child ChunkNode: %d h:%f w:%f res:%d,%d center:%f,%f,%f\n", node->vao_id, chunk_size.x, chunk_size.y, chunk_resolution.x, chunk_resolution.y, node->center.x, node->center.y, node->center.z); 
        }
    }
    void Update(glm::vec3 position) {
       
/*        std::queue<ChunkNode*> nodes;
        nodes.push(root);
        
        while(nodes.size() > 0) {
            ChunkNode* node = nodes.front();
            nodes.pop();
            
            printf("Computing screen space error between %f %f %f & %f %f %f\n", node->center.x,  node->center.x,  node->center.y, 
                   position.x, position.y, position.z);
            if(ComputeScreenSpaceError(node, position) < 1.f) {
                printf("Spliting node\n");
                Split(node);
                for(uint32_t idx = 0; idx < 4; ++idx) {                    
                    nodes.push(&node->children[idx]);
                }
            }
        }
 */       
    }

    void RenderChunkNode(ChunkNode* node) {
        GL_CHECK(glBindVertexArray(node->vao_id));

        GL_CHECK(glActiveTexture(GL_TEXTURE0));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, node->heightmap_tex));
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, node->num_vertices));          
    }
    
     
    void Render(Camera& cam) {
        GL_CHECK(glUseProgram(program));
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(program, "view_proj"), 1, GL_FALSE, glm::value_ptr(cam.GetProjection() * cam.GetView())));                        
        GL_CHECK(glUniform1i(glGetUniformLocation(program, "heightmap"), 0));
//        RenderChunkNode(root);
        RenderChunkNode(&root->children[3]);
        RenderChunkNode(&root->children[1]);
        RenderChunkNode(&root->children[2]);
        RenderChunkNode(&root->children[0].children[0]);
        RenderChunkNode(&root->children[0].children[1]);
        RenderChunkNode(&root->children[0].children[2]);
        RenderChunkNode(&root->children[0].children[3]);
    }

};


#endif

