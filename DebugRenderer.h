#ifndef __debug_renderer_h__
#define __debug_renderer_h__

#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include "GLHelpers.h"
#include <vector>

class DebugRenderer {
private:
    GLuint program;
    
    std::vector<GLuint> lines;
    std::vector<GLuint> points;
private:
    static DebugRenderer* _renderer;
    DebugRenderer() {
        GLuint shaders[2] = { 0 };    
        shaders[0] = gl::CreateShaderFromFile(GL_VERTEX_SHADER, "/Users/eugene.sturm/projects/misc/planet/debug_vs.glsl");        
        shaders[1] = gl::CreateShaderFromFile(GL_FRAGMENT_SHADER, "/Users/eugene.sturm/projects/misc/planet/debug_fs.glsl");    
        program = gl::CreateProgram(shaders, 2);
        GL_CHECK(glLineWidth(0.9f));
        GL_CHECK(glPointSize(5.f));
        
    }
public:
    static DebugRenderer* GetInstance() {
        if(_renderer == NULL) {
            _renderer = new DebugRenderer();            
        }
        return _renderer;
    }    

    void DrawSphere(const glm::vec3& p, float radius, bool depth_enabeled = true) {

    }
    void DrawAxis(const glm::mat4& transform, float scale, bool depth_enabled = true) {

    }

    void DrawRect(const glm::vec3& tl, const glm::vec3& br) {
        float w = br.x - tl.x;
        float h = tl.y - br.y;
        float z_diff = tl.z - br.z;
        glm::vec3 tr = tl + glm::vec3(w, 0, z_diff);
        glm::vec3 bl = tl + glm::vec3(0, -h, z_diff);
        DrawLine(tl, tr);
        DrawLine(tr, br);
        DrawLine(br, bl);
        DrawLine(bl, tl);


    }

    void DrawPoint(const glm::vec3& p) {
        float positions[] = {
            p.x, p.y, p.z,            
        };

        GLuint vertex_buffer = gl::CreateBuffer(GL_ARRAY_BUFFER, positions, sizeof(float) * 6, GL_STATIC_DRAW);        
        GLuint vao_id;
        GL_CHECK(glGenVertexArrays(1, &vao_id));
        GL_CHECK(glBindVertexArray(vao_id));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer));
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0));
        points.push_back(vao_id);
    }
    void DrawCross(const glm::vec3& p, float scale = 1.f, bool depth_enabled = true) {
        DrawLine(p + glm::vec3(-10, 0, 0), p + glm::vec3(10, 0, 0));
        DrawLine(p + glm::vec3(0, -10, 0), p + glm::vec3(0, 10, 0));
        DrawLine(p + glm::vec3(0, 0, -10), p + glm::vec3(0, 0, 10));
    }

    void DrawLine(const glm::vec3& p1, const glm::vec3& p2, float line_width = 1.f, bool depth_enabled = true) {
        float positions[] = {
            p1.x, p1.y, p1.z,
            p2.x, p2.y, p2.z
        };
        GLuint vertex_buffer = gl::CreateBuffer(GL_ARRAY_BUFFER, positions, sizeof(float) * 6, GL_STATIC_DRAW);        
        GLuint vao_id;
        GL_CHECK(glGenVertexArrays(1, &vao_id));
        GL_CHECK(glBindVertexArray(vao_id));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer));
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0));
        lines.push_back(vao_id);
    }

    void DrawTriangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float line_width = 1.f, bool depth_enabled = true) {

    }

    void Render(const glm::mat4& pv) {
        GL_CHECK(glUseProgram(program));
        GLint location = glGetUniformLocation(program, "proj_view");        
        GL_CHECK(glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(pv)));

        for(GLuint id : lines) {            
            GL_CHECK(glBindVertexArray(id));
            GL_CHECK(glDrawArrays(GL_LINES, 0, 2));
        }

        for(GLuint id : points) {            
            GL_CHECK(glBindVertexArray(id));
            GL_CHECK(glDrawArrays(GL_POINTS, 0, 1));
        }
     
    }

};

#endif