#ifndef __terrain_h__
#define __terrain_h__

namespace sturm {

struct BoundingBox {
    BoundingBox(glm::dvec3 min, glm::dvec3 max) : min(min), max(max) {};
    BoundingBox() {};

    glm::dvec3 min;
    glm::dvec3 max;    
};

struct Viewport {
    uint32_t width;
    uint32_t height;
}

struct Frustum {
    glm::vec3 frustum_planes[6];

    bool PointInFrustum(const glm::vec3& p);
    bool BoxInFrustum(const BoundingBox& bbox);
};

Frustum BuildFrustumPlanes(const glm::mat4& projection, const glm::mat4& view) {

};

struct Camera {
    Cam(float fov_degrees = 45.f, float aspect_ratio = 1.33333f, float znear = 0.1f, float zfar = 10000.f);

    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 look;
    glm::dvec3 pos;

    float fov_degrees;
    double zfar;
    double znear;
    float aspect_ratio;

    void Translate(glm::vec3 translation);
    void MoveTo(glm::vec3 pos);
    void Yaw(float degrees);
    void Pitch(float degrees);
    void LookAt(glm::vec3 target);
    
    float GetHorizontalFieldOfView();
    float GetVerticalFieldOfView();
    glm::mat4 BuildView();
    glm::mat4 BuildProjection();    
};

float ComputeVerticalFieldOfView(float fov_degrees, float aspect_ratio) {
    return 0;    
}



double DistanceFromBoundingBox(const BoundingBox& bbox, const glm::vec3& p) {
    glm::vec3 nearest_point_on_bbox = glm::vec3();    
    nearest_point_on_bbox.x = (p.x < bbox.min.x) ? bbox.min.x : (p.x > bbox.max.x) ? bbox.max.x : p.x;
    nearest_point_on_bbox.y = (p.y < bbox.min.y) ? bbox.min.y : (p.y > bbox.max.y) ? bbox.max.y : p.y;
    nearest_point_on_bbox.z = (p.z < bbox.min.z) ? bbox.min.z : ((p.z > bbox.max.z) ? bbox.max.z : p.z);   
    if(nearest_point_on_bbox == p) {
        return 0; // inside/on the bbox
    } else {
        return glm::length(p - nearest_point_on_bbox);    
    }    
}

double ComputeScreenSpaceError(const glm::vec3& eye, float hfov_radians, float viewport_width, const BoundingBox& bbox, float geometric_error) {
    double D = ComputeDistanceFromBoundingBox(eye, bbox);            
    double w = 2.f * D * tan(hfov_radians / 2.f);
    double res = geometric_error * viewport_width / w;        
    return res;   
}

struct TerrainNode {
    BoundingBox bbox;

    double size;
    double wx;
    double wy;

    double tx;
    double ty;
    uint32_t lod_level;

    TerrainNode* parent;
    TerrainNode* children;
};

struct Terrain {    
    uint32_t max_lod;
    double resolution;    
    double size;
    double wx;
    double wy;

    HeightmapGenerator* heightmap_generator;
    NormalmapGenerator* normalmap_generator;
    TerrainNode* root;    
}


struct HeightmapGenerator {    
    std::vector<float> GenerateRegion(double wx, double wy, double wz, double size, uint32_t resolution) {}
}

struct NormalmapGenerator {    
    std::vector<glm::vec3> GenerateNormalmap(std::vector<float> heightmap, double size, uint32_t resolution) {}
}

void Update() {
    std::queue<TerrainNode*> q;
    q.add(root);

    while(q.size() > 0) {
        TerrainNode* node = q.front();
        q.pop();

        double rho = ComputeScreenSpaceError();
        if(rho < tau) {
            if(node->children) {
                // bfs children
            } else {
                // add to split queue
            }            
        } else {
            // has unecessary children
            if(node->children) {
                // add node to be merge queue
            }
        }
    }
}

void Render() {
    // bfs terrain
    // render all leaves
}


struct GPUTile {
    void CopyData();
}

class GPUTileCache {    
    GPUTile* GetTile(uint32_t lod, uint32_t tx, uint32_t ty) {

    }


}
class GPUTileBuffer {    
    std::vector<uint32_t> free_tiles;

    GPUTileBuffer(uint32_t tile_size, uint32_t num_tiles) {        
    }

    GPUTile* NewTile();
    void DeleteTile(GPUTile*);

};

void SplitNode(TerrainNode* node) {
    TerrainNode* children = new TerrainNode[4];

    for(uint32_t idx = 0; idx < 4; ++idx) {
        TerrainNode* node = &children[idx];
        // GenerateHeightMapData();
        // GPUTile* heightmap_tile = gpu_heightmap_tile_buffer.NewTile();
        // heightmap_tile->CopyData();

        // GenerateNormalMapData();    
        // GPUTile* normalmap_tile = gpu_normapmap_tile_buffer.NewTile();
        // normalmap_tile->CopyData();
    }    


    node->children = children;
}





#endif