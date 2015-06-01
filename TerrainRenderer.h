#ifndef __terrain_renderer_h__
#define __terrain_renderer_h__

#include "GLHelpers.h"
#include <glm/glm.hpp>
#include <vector>
#include "Camera.h"
#include "Frustum.h"
#include <noise/noise.h>

struct PlayerViewConstantBuffer {
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 world;
    glm::vec3 eye_pos;

};

class TerrainRenderer {
private:    
    glm::mat4 world;
    GLuint vertex_buffer;
    GLuint index_buffer;
    GLuint constant_buffer;
    GLuint vao_id;
    GLuint heightmap_tex;
    GLuint program;
public:
    TerrainRenderer() {
        noise::module::Perlin gen;

        GLuint shaders[4] = { 0 };    
        shaders[0] = gl::CreateShaderFromFile(GL_VERTEX_SHADER, "/Users/eugene.sturm/projects/misc/planet/planet_vs.glsl");    
        shaders[1] = gl::CreateShaderFromFile(GL_TESS_CONTROL_SHADER, "/Users/eugene.sturm/projects/misc/planet/planet_tcs.glsl");
        shaders[2] = gl::CreateShaderFromFile(GL_TESS_EVALUATION_SHADER, "/Users/eugene.sturm/projects/misc/planet/planet_tes.glsl");
        shaders[3] = gl::CreateShaderFromFile(GL_FRAGMENT_SHADER, "/Users/eugene.sturm/projects/misc/planet/planet_fs.glsl");    
        program = gl::CreateProgram(shaders, 4);
        
        std::vector<glm::vec3> positions;
        std::vector<uint32_t> indices;
        std::vector<float> heightmap_data;

        
        

        float width_in_km = 10000;
        float height_in_km = 10000;
        float w = 56;
        float h = 56;
        float x = -width_in_km / 2.f;        
        float z = -height_in_km / 2.f;
        float dx = width_in_km / w;
        float dz = height_in_km / h;
        for(uint32_t row = 0; row < w; ++row) {
            for(uint32_t col = 0; col < h; ++col) {
                float y = gen.GetValue(x*0.01, 0, z*0.01);
                positions.push_back(glm::vec3(x , y, z));
                x += dx ;         
            }
            x = -width_in_km/2.f;
            z += dz;
        }

        uint32_t heightmap_width = 1024;
        uint32_t heightmap_height = 1024;
        for(uint32_t i = 0; i < heightmap_height; ++i) {
            for(uint32_t j = 0; j < heightmap_height; ++j) {
                heightmap_data.push_back(gen.GetValue(0, 0, 0));
            }
        }
        
        heightmap_tex = gl::CreateTexture2D(GL_R32F, GL_RED, GL_FLOAT, heightmap_width, heightmap_height, heightmap_data.data());
        for(uint32_t row = 0; row < w - 1; ++row) {
            for(uint32_t col = 0; col < h - 1; ++col) {
                indices.push_back((row + 1) * w + col);
                indices.push_back((row + 1) * w + (col + 1));
                indices.push_back((row) * w + (col));
                indices.push_back((row) * w + (col + 1));
            }
            
        }

        
        vertex_buffer = gl::CreateBuffer(GL_ARRAY_BUFFER, positions.data(), sizeof(float) * 3 * positions.size(), GL_STATIC_DRAW);
        index_buffer = gl::CreateBuffer(GL_ELEMENT_ARRAY_BUFFER, indices.data(), sizeof(uint32_t) * indices.size(), GL_STATIC_DRAW);
        constant_buffer = gl::CreateBuffer(GL_UNIFORM_BUFFER, NULL, sizeof(PlayerViewConstantBuffer), GL_DYNAMIC_DRAW);

        // create vertex array object using vertex layout and generated input layout?
        
        GL_CHECK(glGenVertexArrays(1, &vao_id));
        GL_CHECK(glBindVertexArray(vao_id));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer));
        GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer));
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0));        

        GLuint index = glGetUniformBlockIndex(program, "PlayerViewConstantBuffer");    
        GL_CHECK(glUniformBlockBinding(program, index, 0));
        GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, 0, constant_buffer));
        GL_CHECK(glPatchParameteri(GL_PATCH_VERTICES, 4));
    }

    void Render(Camera& cam, Frustum& frustum) {
        GL_CHECK(glBindVertexArray(vao_id));
        GL_CHECK(glUseProgram(program));

        PlayerViewConstantBuffer buffer;//(PlayerViewConstantBuffer*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(PlayerViewConstantBuffer), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        buffer.view = cam.GetView();
        buffer.proj = cam.GetProjection();
        buffer.world = world;
        buffer.eye_pos = cam.GetPos();
//        glUnmapBuffer(GL_UNIFORM_BUFFER);   
        gl::UpdateUniformBuffer(&buffer, sizeof(PlayerViewConstantBuffer));
                                

        GL_CHECK(glDrawElements(GL_PATCHES, 56 * 56 * 4, GL_UNSIGNED_INT, 0));
    }
};

#endif
