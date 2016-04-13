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
#include "glm/detail/type_vec.hpp"
#include "glm/detail/type_vec2.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include FT_FREETYPE_H
#include "glm/glm.hpp"
#include "Config.h"
#include "File.h"
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
    graphics::ShaderHandle _cursorShaders[2];
    graphics::VertexBufferHandle _vertex_buffer, _cursor_vbuffer;
    FT_Pos maxHeight = 0;

    float winWidth = 800.f, winHeight = 600.f;
public:
    TextRenderer(graphics::RenderDevice* render_device) : _render_device(render_device) {
        std::string shaderDirPath = config::Config::getInstance().GetConfigString("RenderDeviceSettings", "ShaderDirectory");
        if (!fs::IsPathDirectory(shaderDirPath)) {
            LOG_D("%s", "Invalid Directory Path given for ShaderDirectory. Attempting default.");
            shaderDirPath = fs::AppendPathProcessDir("/shaders");
        }
        std::string vs_contents = ReadFileContents(shaderDirPath + "/" + render_device->DeviceConfig.DeviceAbbreviation + "/text_vs" + render_device->DeviceConfig.ShaderExtension);
        std::string fs_contents = ReadFileContents(shaderDirPath + "/" + render_device->DeviceConfig.DeviceAbbreviation + "/text_ps" + render_device->DeviceConfig.ShaderExtension);

        // Note(eugene): cleanup shaders
        _shaders[0] = _render_device->CreateShader(graphics::ShaderType::VERTEX_SHADER, vs_contents);
        _shaders[1] = _render_device->CreateShader(graphics::ShaderType::FRAGMENT_SHADER, fs_contents);

        assert(_shaders[0] && _shaders[1]);


        // cursor shaders -- move me
        vs_contents = ReadFileContents(shaderDirPath + "/" + render_device->DeviceConfig.DeviceAbbreviation + "/cursor_vs" + render_device->DeviceConfig.ShaderExtension);
        fs_contents = ReadFileContents(shaderDirPath + "/" + render_device->DeviceConfig.DeviceAbbreviation + "/cursor_ps" + render_device->DeviceConfig.ShaderExtension);

        // Note(eugene): cleanup shaders
        _cursorShaders[0] = _render_device->CreateShader(graphics::ShaderType::VERTEX_SHADER, vs_contents);
        _cursorShaders[1] = _render_device->CreateShader(graphics::ShaderType::FRAGMENT_SHADER, fs_contents);

        assert(_cursorShaders[0] && _cursorShaders[1]);

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
                LOG_D("%s", "ERROR::FREETYTPE: Failed to load Glyph");
                continue;
            }

            graphics::TextureHandle texHandle = 0;

            if (face->glyph->bitmap.width != 0 && face->glyph->bitmap.rows != 0) {
                // Generate texture
                if (face->glyph->bitmap.rows > maxHeight) maxHeight = face->glyph->bitmap.rows;

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
        _vertex_buffer = _render_device->CreateVertexBuffer(layout, 0, graphics::SizeofParam(graphics::ParamType::Float4) * 6, graphics::BufferUsage::DYNAMIC);
        _cursor_vbuffer = _render_device->CreateVertexBuffer(layout, 0, graphics::SizeofParam(graphics::ParamType::Float4) * 2, graphics::BufferUsage::DYNAMIC);
    }

    void SetRenderWindowSize(uint32_t width, uint32_t height) {
        winWidth = (float)width;
        winHeight = (float)height;
    }

    void RenderText(std::string text, float x, float y, float scale, glm::vec3 color) {
        graphics::BlendState blend_state;
        blend_state.enable = true;
        blend_state.src_rgb_func = graphics::BlendFunc::SRC_ALPHA;
        blend_state.src_alpha_func = blend_state.src_rgb_func;
        blend_state.dst_rgb_func = graphics::BlendFunc::ONE_MINUS_SRC_ALPHA;
        blend_state.dst_alpha_func = blend_state.dst_rgb_func;
        _render_device->SetBlendState(blend_state);
        _render_device->SetRasterState(graphics::RasterState());
        _render_device->SetDepthState(graphics::DepthState());

        glm::mat4 projection = glm::ortho(0.0f, winWidth, 0.0f, winHeight);
        // Activate corresponding render state  
        _render_device->SetVertexShader(_shaders[0]);
        _render_device->SetPixelShader(_shaders[1]);

        float textColor[3] = { color.x, color.y, color.z };
        _render_device->SetShaderParameter(_shaders[1], graphics::ParamType::Float3, "textColor", &textColor);
        _render_device->SetShaderParameter(_shaders[0], graphics::ParamType::Float4x4, "projection", &projection);

        // Iterate through all characters
        std::string::const_iterator c;
        for (c = text.begin(); c != text.end(); c++) {
            Character ch = characters[*c];

            float xpos = x + ch.bearing.x * scale;
            float ypos = y - (ch.size.y - ch.bearing.y) * scale;

            // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)

            // handle 'space'
            if (*c == ' ')
                continue;

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

            _render_device->SetShaderTexture(_shaders[1], ch.texture_id, graphics::TextureSlot::BASE);
            _render_device->SetVertexBuffer(_vertex_buffer);
            // Update content of VBO memory
            _render_device->UpdateVertexBuffer(_vertex_buffer, vertices, sizeof(vertices));

            // Render quad
            _render_device->DrawPrimitive(graphics::PrimitiveType::TRIANGLES, 0, 6);
        }
    }

    void RenderCursor(std::string text, uint32_t cursorPosition, float x, float y, float scale, glm::vec3 color) {
        if (cursorPosition > text.length()) {
            LOG_D("%s", "CursorRender: CursorPos larger than string, assuming end of text.");
            cursorPosition = text.length();
        }

        graphics::BlendState blend_state;
        blend_state.enable = true;
        blend_state.src_rgb_func = graphics::BlendFunc::SRC_ALPHA;
        blend_state.src_alpha_func = blend_state.src_rgb_func;
        blend_state.dst_rgb_func = graphics::BlendFunc::ONE_MINUS_SRC_ALPHA;
        blend_state.dst_alpha_func = blend_state.dst_rgb_func;
        _render_device->SetBlendState(blend_state);
        _render_device->SetRasterState(graphics::RasterState());
        _render_device->SetDepthState(graphics::DepthState());

        glm::mat4 projection = glm::ortho(0.0f, winWidth, 0.0f, winHeight);

        _render_device->SetVertexShader(_cursorShaders[0]);
        _render_device->SetPixelShader(_cursorShaders[1]);

        _render_device->SetShaderParameter(_cursorShaders[1], graphics::ParamType::Float3, "textColor", &color);
        _render_device->SetShaderParameter(_cursorShaders[0], graphics::ParamType::Float4x4, "projection", &projection);

        float cursorX = x;
        for (uint32_t j = 0; j < cursorPosition; ++j) {
            Character ch = characters[text[j]];
            // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            cursorX += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
        }

        float vertices[2][4] = {
            { cursorX, y,             0.f, 0.f},
            { cursorX, y + maxHeight, 0.f, 0.f}
        };

        _render_device->SetVertexBuffer(_cursor_vbuffer);
        _render_device->UpdateVertexBuffer(_cursor_vbuffer, vertices, sizeof(vertices));
        _render_device->DrawPrimitive(graphics::PrimitiveType::LINESTRIP, 0, 2);
    }
};


#endif
