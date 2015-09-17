#ifndef __text_renderer_h__
#define __text_renderer_h__

#include "gfx/RenderDevice.h"
#include "GLHelpers.h"
#include <ft2build.h>
#include <glm/detail/type_vec.hpp>
#include <glm/detail/type_vec2.hpp>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <map>

class TextRenderer {
private:
    struct Character {
        GLuint     texture_id;  
        glm::ivec2 size;       
        glm::ivec2 bearing;    
        GLuint     advance;  
    };
    gfx::RenderDevice* _render_device;
    std::map<GLchar, Character> characters;

    GLuint _shaders[2];
    GLuint VAO, VBO;
public:
    TextRenderer(gfx::RenderDevice* render_device) : _render_device(render_device) {
        std::string vs_contents = ReadFileContents(fs::AppendPathProcessDir("../shaders/" + render_device->DeviceConfig.DeviceAbbreviation + "/text_vs" + render_device->DeviceConfig.ShaderExtension));
        const char* vs_src = vs_contents.c_str();

        std::string fs_contents = ReadFileContents(fs::AppendPathProcessDir("../shaders/" + render_device->DeviceConfig.DeviceAbbreviation + "/text_ps" + render_device->DeviceConfig.ShaderExtension));
        const char* fs_src = fs_contents.c_str();

        // Note(eugene): cleanup shaders
        _shaders[0] = _render_device->CreateShader(graphics::ShaderType::VERTEX_SHADER, &vs_src);
        _shaders[1] = _render_device->CreateShader(graphics::ShaderType::FRAGMENT_SHADER, &fs_src);

        assert(_shaders[0] && _shaders[1]);

        FT_Library ft;
        if (FT_Init_FreeType(&ft))
            std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

        FT_Face face;
        if (FT_New_Face(ft, "/Library/Fonts/Arial.ttf", 0, &face))
            std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;  

        FT_Set_Pixel_Sizes(face, 0, 48); 

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

        for (GLubyte c = 0; c < 128; c++) {
            // Load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // Generate texture
            _render_device->CreateTexture2D(gfx::TextureFormat::R_UBYTE)
            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
                );
            // Set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // Now store character for later use
            Character character = {
                texture, 
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<GLuint>(face->glyph->advance.x)
            };
            characters.insert(std::pair<GLchar, Character>(c, character));
        }

        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0); 
    }

    void RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
        // Activate corresponding render state  
        glUseProgram(_program);

        glUniform3f(glGetUniformLocation(_program, "textColor"), color.x, color.y, color.z);
        gl::SetUniform(_program, "projection", ParamType::Float4x4, &projection);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);

        // Iterate through all characters
        std::string::const_iterator c;
        for (c = text.begin(); c != text.end(); c++) {
            Character ch = characters[*c];

            GLfloat xpos = x + ch.bearing.x * scale;
            GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;

            GLfloat w = ch.size.x * scale;
            GLfloat h = ch.size.y * scale;
            // Update VBO for each character
            GLfloat vertices[6][4] = {
                { xpos,     ypos + h,   0.0, 0.0 },            
                { xpos,     ypos,       0.0, 1.0 },
                { xpos + w, ypos,       1.0, 1.0 },

                { xpos,     ypos + h,   0.0, 0.0 },
                { xpos + w, ypos,       1.0, 1.0 },
                { xpos + w, ypos + h,   1.0, 0.0 }           
            };
            // Render glyph texture over quad
            glBindTexture(GL_TEXTURE_2D, ch.texture_id);
            // Update content of VBO memory
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            // Render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }


};


#endif
