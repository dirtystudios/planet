#pragma once

// TODO::Dont use map -- just use switch statements
#include <map>
#include <string>
#include <cassert>
#ifdef _WIN32
#else
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#endif
#include "BufferType.h"
#include "BufferUsage.h"
#include "ParamType.h"
#include "BlendMode.h"
#include "CullMode.h"
#include "DepthWriteMask.h"
#include "FillMode.h"
#include "BlendFunc.h"
#include "BlendState.h"
#include "WindingOrder.h"
#include "TextureFormat.h"
#include "GLTextureFormatDesc.h"
#include "Helpers.h"
#include "PrimitiveType.h"
#include "GLStructs.h"
#include "RasterState.h"
#include "DepthFunc.h"
#include "DepthState.h"
#include "VertexLayoutDesc.h"
#include "BufferAccess.h"

namespace graphics {
class GLEnumAdapter {
public:
    static BufferType ConvertBufferType(GLenum enumIn) {
        switch (enumIn) {
        case GL_ARRAY_BUFFER:
            return BufferType::VertexBuffer;
        case GL_ELEMENT_ARRAY_BUFFER:
            return BufferType::IndexBuffer;
        case GL_UNIFORM_BUFFER:
                return BufferType::ConstantBuffer;
        default:
            assert(false);
        }
    }
    
    static GLenum Convert(BufferAccess enumIn) {
        switch (enumIn) {
            case BufferAccess::Read:
                return GL_READ_ONLY;
            case BufferAccess::Write:
                return GL_WRITE_ONLY;
            case BufferAccess::ReadWrite:
                return GL_READ_WRITE;
            default:
                assert(false);
        }
    }

    static GLenum Convert(BufferType enumIn) {
        switch (enumIn) {
        case BufferType::VertexBuffer:
            return GL_ARRAY_BUFFER;
        case BufferType::IndexBuffer:
            return GL_ELEMENT_ARRAY_BUFFER;
        case BufferType::ConstantBuffer:
            return GL_UNIFORM_BUFFER;
        default:
            assert(false);
        }
    }

    static GLenum Convert(BufferUsage enumIn) {
        assert(enumIn != BufferUsage::Count);
        static std::map<BufferUsage, GLenum> bufferUsageMapping = {
            {BufferUsage::Static, GL_STATIC_DRAW}, {BufferUsage::Dynamic, GL_DYNAMIC_DRAW},
        };

        return bufferUsageMapping[enumIn];
    }

    static GLenum Convert(ShaderType enumIn) {
        assert(enumIn != ShaderType::Count);
        switch (enumIn) {
        case ShaderType::VertexShader:
            return GL_VERTEX_SHADER;
        case ShaderType::PixelShader:
            return GL_FRAGMENT_SHADER;
        default:
            assert(false);
        }
    }

    static ShaderType ConvertShaderType(GLenum enumIn) {
        switch (enumIn) {
        case GL_VERTEX_SHADER:
            return ShaderType::VertexShader;
        case GL_FRAGMENT_SHADER:
            return ShaderType::PixelShader;
        default:
            assert(false);
        }
    }

    static GLenum Convert(PrimitiveType enumIn) {
        assert(enumIn != PrimitiveType::Count);
        static std::map<PrimitiveType, GLenum> primitiveTypeMapping = {
            {PrimitiveType::Triangles, GL_TRIANGLES},
            {PrimitiveType::Lines, GL_LINES}
        };

        return primitiveTypeMapping[enumIn];
    }

    static GLenum Convert(ParamType enumIn) {
        assert(enumIn != ParamType::Count);
        switch (enumIn) {
        case ParamType::Int32:
            return GL_INT;
        case ParamType::UInt32:
            return GL_UNSIGNED_INT;
        case ParamType::Float:
            return GL_FLOAT;
        case ParamType::Float2:
            return GL_FLOAT_VEC2;
        case ParamType::Float3:
            return GL_FLOAT_VEC3;
        case ParamType::Float4:
            return GL_FLOAT_VEC4;
        case ParamType::Float4x4:
            return GL_FLOAT_MAT4;
        default:
            assert(false);
        }
    }

    static ParamType ConvertParamType(GLenum enumIn) {
        switch (enumIn) {
        case GL_INT:
            return ParamType::Int32;
        case GL_UNSIGNED_INT:
            return ParamType::UInt32;
        case GL_FLOAT:
            return ParamType::Float;
        case GL_FLOAT_VEC2:
            return ParamType::Float2;
        case GL_FLOAT_VEC3:
            return ParamType::Float3;
        case GL_FLOAT_VEC4:
            return ParamType::Float4;
        case GL_FLOAT_MAT4:
            return ParamType::Float4x4;
        default:
            assert(false);
        }
    }

    static GLTextureFormatDesc Convert(TextureFormat enumIn) {
        assert(enumIn != TextureFormat::Count);
        static std::map<TextureFormat, GLTextureFormatDesc> textureFormatMapping = {
            {TextureFormat::R32F, {GL_R32F, GL_FLOAT, GL_RED}},
            {TextureFormat::RGB32F, {GL_RGB32F, GL_FLOAT, GL_RGB}},
            {TextureFormat::RGBA32F, {GL_RGBA32F, GL_FLOAT, GL_RGBA}},
            {TextureFormat::R_U8, {GL_RED, GL_UNSIGNED_BYTE, GL_RED}},
            {TextureFormat::RGB_U8, {GL_RGB, GL_UNSIGNED_BYTE, GL_RGB}},
        };

        return textureFormatMapping[enumIn];
    }

    static GLenum Convert(BlendMode enumIn) {
        assert(enumIn != BlendMode::Count);
        static std::map<BlendMode, GLenum> blendModeMapping = {
            {BlendMode::Add, GL_FUNC_ADD},
            {BlendMode::Subtract, GL_FUNC_SUBTRACT},
            {BlendMode::ReverseSubtract, GL_FUNC_REVERSE_SUBTRACT},
            {BlendMode::Min, GL_MIN},
            {BlendMode::Max, GL_MAX},
        };

        return blendModeMapping[enumIn];
    }

    static GLenum Convert(CullMode enumIn) {
        assert(enumIn != CullMode::Count);
        static std::map<CullMode, GLenum> cullModeMapping = {
            {CullMode::None, GL_FALSE}, {CullMode::Front, GL_FRONT}, {CullMode::Back, GL_BACK},
        };

        return cullModeMapping[enumIn];
    }

    static GLenum Convert(WindingOrder enumIn) {
        assert(enumIn != WindingOrder::Count);
        static std::map<WindingOrder, GLenum> windingOrderMapping = {
            {WindingOrder::FrontCCW, GL_CCW}, {WindingOrder::FrontCW, GL_CW},
        };

        return windingOrderMapping[enumIn];
    }

    static GLenum Convert(DepthWriteMask enumIn) {
        assert(enumIn != DepthWriteMask::Count);
        static std::map<DepthWriteMask, GLenum> depthWriteMaskMapping = {
            {DepthWriteMask::Zero, GL_FALSE}, {DepthWriteMask::All, GL_TRUE},
        };

        return depthWriteMaskMapping[enumIn];
    }

    static GLenum Convert(DepthFunc enumIn) {
        assert(enumIn != DepthFunc::Count);
        static std::map<DepthFunc, GLenum> depthFunceMaskMapping = {
            {DepthFunc::Never, GL_NEVER},         {DepthFunc::Less, GL_LESS},       {DepthFunc::Equal, GL_EQUAL},
            {DepthFunc::LessEqual, GL_LEQUAL},    {DepthFunc::Greater, GL_GREATER}, {DepthFunc::NotEqual, GL_NOTEQUAL},
            {DepthFunc::GreaterEqual, GL_GEQUAL}, {DepthFunc::Always, GL_ALWAYS},
        };

        return depthFunceMaskMapping[enumIn];
    }

    static GLenum Convert(FillMode enumIn) {
        assert(enumIn != FillMode::Count);
        static std::map<FillMode, GLenum> fillModeMapping = {
            {FillMode::Wireframe, GL_LINE}, {FillMode::Solid, GL_FILL},
        };

        return fillModeMapping[enumIn];
    }

    static GLVertexElement Convert(const VertexLayoutElement& element) {
        GLVertexElement glElement;
        switch (element.type) {
        case VertexAttributeType::Float4:
            return {GL_FLOAT, 4};
        case VertexAttributeType::Float3:
            return {GL_FLOAT, 3};
        case VertexAttributeType::Float2:
            return {GL_FLOAT, 2};
        case VertexAttributeType::Float:
            return {GL_FLOAT, 1};
        default:
            assert(false);
        }
    }

     static GLVertexElement Convert(const GLAttributeMetadata& attribute) {        
        assert(attribute.size == 1);
        switch (attribute.type) {                        
            case GL_FLOAT:  return { GL_FLOAT, 1 };
            case GL_FLOAT_VEC2: return { GL_FLOAT, 2 };
            case GL_FLOAT_VEC3: return { GL_FLOAT, 3 };
            case GL_FLOAT_VEC4: return { GL_FLOAT, 4 };
            default:        assert(false);
        }
    }

    static GLenum Convert(BlendFunc enumIn) {
        assert(enumIn != BlendFunc::Count);
        static std::map<BlendFunc, GLenum> blendFunceMaskMapping = {
            {BlendFunc::Zero, GL_ZERO},
            {BlendFunc::One, GL_ONE},
            {BlendFunc::SrcColor, GL_SRC_COLOR},
            {BlendFunc::OneMinusSrcColor, GL_ONE_MINUS_SRC_COLOR},
            {BlendFunc::SrcAlpha, GL_SRC_ALPHA},
            {BlendFunc::OneMinusSrcAlpha, GL_ONE_MINUS_SRC_ALPHA},
            {BlendFunc::DstAlpha, GL_DST_ALPHA},
            {BlendFunc::OneMinusDstAlpha, GL_ONE_MINUS_DST_ALPHA},
            {BlendFunc::ConstantColor, GL_CONSTANT_COLOR},
            {BlendFunc::OneMinusConstantColor, GL_ONE_MINUS_CONSTANT_COLOR},
            {BlendFunc::ConstantAlpha, GL_CONSTANT_ALPHA},
            {BlendFunc::OneMinusConstantAlpha, GL_ONE_MINUS_CONSTANT_ALPHA},
            {BlendFunc::SrcAlphaSaturate, GL_SRC_ALPHA_SATURATE},
            {BlendFunc::Src1Color, GL_SRC1_COLOR},
            {BlendFunc::OneMinusSrc1Color, GL_ONE_MINUS_SRC1_COLOR},
            {BlendFunc::Src1Alpha, GL_SRC1_ALPHA},
            {BlendFunc::OneMinusSrc1Alpha, GL_ONE_MINUS_SRC1_ALPHA},
        };

        return blendFunceMaskMapping[enumIn];
    }

    static GLBlendState Convert(const BlendState& blendState) {
        GLBlendState glBlendState;
        glBlendState.enable       = blendState.enable;
        glBlendState.srcRgbFunc   = GLEnumAdapter::Convert(blendState.srcRgbFunc);
        glBlendState.srcAlphaFunc = GLEnumAdapter::Convert(blendState.srcAlphaFunc);
        glBlendState.dstRgbFunc   = GLEnumAdapter::Convert(blendState.dstRgbFunc);
        glBlendState.dstAlphaFunc = GLEnumAdapter::Convert(blendState.dstAlphaFunc);
        glBlendState.rgbMode      = GLEnumAdapter::Convert(blendState.rgbMode);
        glBlendState.alphaMode    = GLEnumAdapter::Convert(blendState.alphaMode);

        return glBlendState;
    }

    static GLRasterState Convert(const RasterState& rasterState) {
        GLRasterState glRasterState;
        glRasterState.fillMode     = GLEnumAdapter::Convert(rasterState.fillMode);
        glRasterState.cullMode     = GLEnumAdapter::Convert(rasterState.cullMode);
        glRasterState.windingOrder = GLEnumAdapter::Convert(rasterState.windingOrder);

        return glRasterState;
    }

    static GLDepthState Convert(const DepthState& depthState) {
        GLDepthState glDepthState;
        glDepthState.enable         = depthState.enable;
        glDepthState.depthWriteMask = GLEnumAdapter::Convert(depthState.depthWriteMask);
        glDepthState.depthFunc      = GLEnumAdapter::Convert(depthState.depthFunc);

        return glDepthState;
    }

    static size_t Hash(const std::vector<GLenum>& enums) {
        size_t hash = 0;
        for (const GLenum& e : enums) {
            HashCombine(hash, e);
        }
        return hash;
    }

    static std::string Convert(GLenum glEnum) {
#define STRINGIFY(x) #x
#define GLENUM(_ty)                                                                                                    \
    case _ty:                                                                                                          \
        return #_ty

        switch (glEnum) {
            GLENUM(GL_TEXTURE);
            GLENUM(GL_RENDERBUFFER);

            GLENUM(GL_INVALID_ENUM);
            GLENUM(GL_INVALID_VALUE);
            GLENUM(GL_INVALID_OPERATION);
            GLENUM(GL_OUT_OF_MEMORY);

            GLENUM(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
            GLENUM(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
            //          GLENUM(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
            //          GLENUM(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
            GLENUM(GL_FRAMEBUFFER_UNSUPPORTED);
            GLENUM(GL_FLOAT);
            GLENUM(GL_FLOAT_VEC2);
            GLENUM(GL_FLOAT_VEC3);
            GLENUM(GL_FLOAT_VEC4);
            GLENUM(GL_FLOAT_MAT2);
            GLENUM(GL_FLOAT_MAT3);
            GLENUM(GL_FLOAT_MAT4);
            GLENUM(GL_INT);
            GLENUM(GL_INT_VEC2);
            GLENUM(GL_INT_VEC3);
            GLENUM(GL_INT_VEC4);
            GLENUM(GL_UNSIGNED_INT);
            GLENUM(GL_UNSIGNED_INT_VEC2);
            GLENUM(GL_UNSIGNED_INT_VEC3);
            GLENUM(GL_SAMPLER_2D);
            GLENUM(GL_SAMPLER_2D_ARRAY);
            GLENUM(GL_SAMPLER_CUBE);
            GLENUM(GL_RGBA);
            GLENUM(GL_RGB);
            GLENUM(GL_UNSIGNED_BYTE);
        }
        return "UNKNOWN GLENUM";
    }

    static std::string ToString(const GLTextureFormatDesc& desc) {
        return "GLTextureFormatDesc [internalFormat:" + Convert(desc.internalFormat) + ", dataFormat:" + Convert(desc.dataFormat) + ", dataType:" + Convert(desc.dataType) + "]";
    }
};
}
