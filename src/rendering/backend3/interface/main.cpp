////
////  main.cpp
////  te
////
////  Created by Eugene Sturm on 9/29/15.
////  Copyright (c) 2015 Eugene Sturm. All rights reserved.
////
//
//#include <iostream>
//#include "GLFW/glfw3.h"
//#include "RenderDevice.h"
//#include "GLDevice.h"
//
// const char* shader = "#version 410 core\n\n\nlayout(std140) uniform PerViewBuffer {\n    mat4 "
//                     "viewz;\n    mat4 projz;\n    vec3 eye_posz;\n};\n\nlayout(std140) "
//                     "uniform PerTerrainBuffer {\n    mat4 worlda;\n    int "
//                     "elevations_tile_indexa;\n    int normals_tile_indexa;\n};\n\n\n\nuniform "
//                     "mat4 view;\nuniform mat4 proj;\nuniform mat4 world;\nuniform int "
//                     "elevations_tile_index;\nuniform int normals_tile_index;\nuniform "
//                     "sampler2DArray base_texture;\nuniform sampler2DArray "
//                     "normal_texture;\n\n\nlayout(location = 0) in vec2 "
//                     "position;\nlayout(location = 1) in vec2 tex;\nlayout(location = 3) out "
//                     "vec3 Normal;\n\nout gl_PerVertex {\n  vec4 gl_Position;\n};\n\nvoid "
//                     "main(void) {\n    float height = 250.f * texture(base_texture, vec3(tex, "
//                     "float(elevations_tile_index))).x;\n    vec3 normal = "
//                     "texture(normal_texture, vec3(tex, float(normals_tile_index))).xyz;\n    "
//                     "vec4 pos = vec4(position.x, position.y, height, 1.f);\n    Normal = "
//                     "normal;    \n\n    /*\n    // in order to sphereicalize the cube, need to "
//                     "be -1 > x > 1, -1 > y > 1, -1 > z > 1\n    float radius = 500\n    vec4 "
//                     "mapping = pos;\n\n    float x_squared = pos.x * pos.x;\n    float "
//                     "y_squared = pos.y * pos.y;\n    float z_squared = pos.z * pos.z;\n    "
//                     "mapping.x = radius * pos.x * sqrt(1.f - (y_squared / 2.f) - (z_squared / "
//                     "2.f) + (y_squared * z_squared / 3.f));\n    mapping.y = radius * pos.y * "
//                     "sqrt(1.f - (z_squared / 2.f) - (x_squared / 2.f) + (z_squared * x_squared "
//                     "/ 3.f));\n    mapping.z = (radius + height) * pos.z * sqrt(1.f - "
//                     "(x_squared / 2.f) - (y_squared / 2.f) + (x_squared * y_squared / 3.f));\n "
//                     "   */\n\n    gl_Position = proj * viewz * worlda * pos;\n}\n";
//
// int main(int argc, const char* argv[]) {
//
//
//
//    // printf("asdf:%lu\n",sizeof(ClearBackBuffer));
//
//    // graphics::OpenGLDriver driver;
//
//    // CommandBuffer::Iterator extractor(buffer);
//    // GLRenderCommand* cmd = nullptr;
//    // while( (cmd = extractor->NextCommand()) != nullptr) {
//    //     delegateMap[cmd->type](cmd);
//    // }
//
//    GLFWwindow* window;
//    if (!glfwInit())
//        exit(EXIT_FAILURE);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
//    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
//    if (!window) {
//        glfwTerminate();
//        exit(EXIT_FAILURE);
//    }
//    glfwMakeContextCurrent(window);
//    glfwSwapInterval(1);
//
//    RenderDevice* device = new GLDevice();
//
//    // graphics::BufferHandle vb_handle;
//    // graphics::ShaderParamHandle param;
//    float mat[16];
//
//    device->InitializeDevice(nullptr, 1, 1);
//    device->PrintDisplayAdapterInfo();
//    BufferId vertexBuffer = device->CreateBuffer(BufferType::VertexBuffer, nullptr, 0, BufferUsage::Static);
//    ShaderId vs     = device->CreateShader(ShaderType::VertexShader, &shader);
//
//    PipelineStateDesc psd;
//    psd.vertexShader = vs;
//    psd.pixelShader = 0;
//    psd.primitiveTopology = PrimitiveType::Triangles;
//
//    PipelineStateId = device->CreatePipelineState(psd):
//
//    ShaderParamId param = device->CreateShaderParam(shaderId, "world", ParamType::Float4x4);
//    device->DestroyShader(shaderId);
//
//    CommandBuffer* cmdBuffer = device->CreateCommandBuffer();
//    for (uint32_t i = 0; i < 50; ++i) {
//        cmdBuffer->ResetCommandBuffer();
//        cmdBuffer->
//        cmdBuffer->SetPipelineState(pipelineStateId);
//        cmdBuffer->SetShaderParam(param, &mat, sizeof(float) * 16);
//        cmdBuffer->DrawPrimitve(vertexBuffer, 0, 0);
//        device->Execute(&cmdBuffer, 1);
//    }
//
//    return 0;
//}
