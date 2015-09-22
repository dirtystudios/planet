#ifndef __text_renderer_h__
#define __text_renderer_h__

#ifdef _WIN32
#include <GL/glew.h>
#else
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#endif

#include "RenderDevice.h"
#include <ft2build.h>
#include <glm/detail/type_vec.hpp>
#include <glm/detail/type_vec2.hpp>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <map>

#include "Log.h"

class TextRenderer {
private:
    struct Character {
        graphics::TextureHandle  texture_id;  
        glm::ivec2               size;       
        glm::ivec2               bearing;    
        uint32_t                 advance;  
    };
    graphics::RenderDevice* _render_device;
    std::map<char, Character> characters;

    graphics::ShaderHandle _shaders[2];
    //GLuint VAO, VBO;
    graphics::VertexBufferHandle VBO;
public:
    TextRenderer(graphics::RenderDevice* render_device) : _render_device(render_device) {
        std::string vs_contents = ReadFileContents(fs::AppendPathProcessDir("../src/rendering/shaders/" + render_device->DeviceConfig.DeviceAbbreviation + "/text_vs" + render_device->DeviceConfig.ShaderExtension));
        const char* vs_src = vs_contents.c_str();

        std::string fs_contents = ReadFileContents(fs::AppendPathProcessDir("../src/rendering/shaders/" + render_device->DeviceConfig.DeviceAbbreviation + "/text_ps" + render_device->DeviceConfig.ShaderExtension));
        const char* fs_src = fs_contents.c_str();

        // Note(eugene): cleanup shaders
        _shaders[0] = _render_device->CreateShader(graphics::ShaderType::VERTEX_SHADER, &vs_src);
        _shaders[1] = _render_device->CreateShader(graphics::ShaderType::FRAGMENT_SHADER, &fs_src);

        assert(_shaders[0] && _shaders[1]);

        FT_Library ft;
        int ftStatus = FT_Init_FreeType(&ft);
        if (ftStatus) {
            LOG_E("FREETYPE: Could not init FreeType Library. Error: %d", ftStatus);
        }

        FT_Face face;
#ifdef _WIN32
        ftStatus = FT_New_Face(ft, "C:/Windows/Fonts/Arial.ttf", 0, &face);
#else
        ftStatus = FT_New_Face(ft, "/Library/Fonts/Arial.ttf", 0, &face);
#endif
        if (ftStatus) {
            LOG_E("FREETYPE: Failed to load font. Error: %d", ftStatus);
        }

        FT_Set_Pixel_Sizes(face, 0, 48); 

        //glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

        for (unsigned char c = 0; c < 128; c++) {
            // Load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                LOG_D("ERROR::FREETYTPE: Failed to load Glyph");
                continue;
            }

            graphics::TextureHandle texHandle = 0;

            if (face->glyph->bitmap.width != 0 && face->glyph->bitmap.rows != 0) {
                // Generate texture
                texHandle = _render_device->CreateTexture2D(graphics::TextureFormat::R_UBYTE,
                                                            face->glyph->bitmap.width,
                                                            face->glyph->bitmap.rows,
                                                            face->glyph->bitmap.buffer);
            }

            // Now store character for later use
            Character character = {
                texHandle, 
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<GLuint>(face->glyph->advance.x)
            };
            characters.insert(std::pair<GLchar, Character>(c, character));
        }

        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        graphics::VertLayout layout;
        layout.Add(graphics::ParamType::Float4);
        VBO = _render_device->CreateVertexBuffer(layout, 0, graphics::SizeofParam(graphics::ParamType::Float4) * 6, graphics::BufferUsage::DYNAMIC);
        /*glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0); */
    }

    void RenderText(std::string text, float x, float y, float scale, glm::vec3 color) {
        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
        // Activate corresponding render state  
        //glUseProgram(_program);
        _render_device->SetVertexShader(_shaders[0]);
        _render_device->SetPixelShader(_shaders[1]);

        float textColor[3] = { color.x, color.y, color.z };
        _render_device->SetShaderParameter(_shaders[1], graphics::ParamType::Float3, "textColor", &textColor);
        _render_device->SetShaderParameter(_shaders[0], graphics::ParamType::Float4x4, "projection", &projection);

        //glUniform3f(glGetUniformLocation(_program, "textColor"), color.x, color.y, color.z);
        //gl::SetUniform(_program, "projection", ParamType::Float4x4, &projection);
        //glActiveTexture(GL_TEXTURE0);
        //glBindVertexArray(VAO);

        // Iterate through all characters
        std::string::const_iterator c;
        for (c = text.begin(); c != text.end(); c++) {
            Character ch = characters[*c];

            float xpos = x + ch.bearing.x * scale;
            float ypos = y - (ch.size.y - ch.bearing.y) * scale;

            float w = ch.size.x * scale;
            float h = ch.size.y * scale;
            // Update VBO for each character
            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0, 0.0 },            
                { xpos,     ypos,       0.0, 1.0 },
                { xpos + w, ypos,       1.0, 1.0 },

                { xpos,     ypos + h,   0.0, 0.0 },
                { xpos + w, ypos,       1.0, 1.0 },
                { xpos + w, ypos + h,   1.0, 0.0 }           
            };
            // Render glyph texture over quad
            //glBindTexture(GL_TEXTURE_2D, ch.texture_id);
            _render_device->SetShaderTexture(_shaders[1], ch.texture_id, graphics::TextureSlot::BASE);
            _render_device->SetVertexBuffer(VBO);
            // Update content of VBO memory
            _render_device->UpdateVertexBuffer(VBO, vertices, sizeof(vertices));

            /*glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
            glBindBuffer(GL_ARRAY_BUFFER, 0); */

            // Render quad
            /*glDrawArrays(GL_TRIANGLES, 0, 6);*/
            _render_device->DrawPrimitive(graphics::PrimitiveType::TRIANGLES, 0, 6);

            // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
        }
        /*glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);*/
    }


};


#endif
