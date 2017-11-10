// blinn_vertex
#version 410 core

// attribute
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_norm;
layout (location = 2) in vec2 a_tex;
layout (location = 3) in uvec4 a_boneIds;
layout (location = 4) in vec4 a_boneWeights;

// constant buffers
layout(std140) uniform _b0_viewConstants {
    vec3 b0_eyePos;
    mat4 b0_view;
    mat4 b0_proj;
};

layout(std140) uniform _b1_objectConstants {
    mat4 b1_world;    
	mat4 b1_boneOffsets[255];
};

// output
layout (location = 0) out vec3 o_normal;
layout (location = 1) out vec3 o_toCamera;
layout (location = 2) out vec2 o_tex;

out gl_PerVertex {
    vec4 gl_Position;
};

////////////////////////////////////////////////////////

void main() {	
    mat4 BoneTransform = b1_boneOffsets[a_boneIds.x] * a_boneWeights.x;
    BoneTransform     += b1_boneOffsets[a_boneIds.y] * a_boneWeights.y;
    BoneTransform     += b1_boneOffsets[a_boneIds.z] * a_boneWeights.z;
    BoneTransform     += b1_boneOffsets[a_boneIds.w] * a_boneWeights.w;

    vec4 worldPosition = b1_world * ( vec4(a_pos, 1.0) * BoneTransform);

    o_tex = a_tex;
    o_normal = a_norm; //normalize((invWV * vec4(norm, 1.0)).xyz);
    o_toCamera = normalize(b0_eyePos - worldPosition.xyz);
	
	gl_Position = (b0_proj * b0_view) * worldPosition;
}