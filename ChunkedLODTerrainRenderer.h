#ifndef __chunkded_lod_terrain_renderer_h__
#define __chunkded_lod_terrain_renderer_h__

struct ChunkedLODTerrainDesc {
    uint32_t max_lod;

    // Note(eugene): hierarchical transforms
    //Transform transform;
    uint32_t size;

    double x;
    double y;

    std::function<double(double x, double y, double z)> i;
};


struct ChunkedLODTerrainNode {
    uint32_t lod;
    uint32_t tx;
    uint32_t ty;

    double x;
    double y;

    BoundingBox bbox;
    float size;

    ChunkedLODTerrainNode* children = { NULL };
};

struct ChunkedLODTerrainHandle {

};

class ChunkedLODTerrainRenderer {
private:
    static const uint32_t k_terrin_quad_resolution = 64;
    static const float k_split_factor = 1.5f;
    static const uint32_t k_max_lod = 7;
    static const uint32_t k_num_quad_tree_nodes = 4;

    std::map<ChunkedLODTerrainNode*, ChunkedLODTerrainDesc> _terrain_descs;
    std::vector<ChunkeLODTerrainNode*> _root_nodes;
public:
    ChunkedLODTerrainHandle RegisterTerrain(const ChunkedLODTerrainDesc& desc) {        
        root = new ChunkedLODTerrainNode();
        root->lod = 0;
        root->tx = 0;
        root->ty = 0;
        root->x = desc.x;
        root->y = desc.y;
        root->size = desc.size
        float half_size = root->size / 2.f;
        // z-coordinates will be filled in when heightmap data is generated
        root->bbox.max = glm::vec3(half_size, half_size, 0); 
        root->bbox.min = glm::vec3(-half_size, -half_size, 0);

        _terrain_desc.insert(desc, root);
        _root_nodes.push_back(root);

        return ChunkdLODTerrainHandle();
    }

    bool UnregisterTerrain(const ChunkedLODTerrainHandle& handle) {

    }

    void Render(const Camera& camera, const Frustum& frustum) {
        for(ChunkedLODTerrainNode* root : _root_nodes) {
            std::queue<ChunkedLODTerrainNode*> dfs_queue;
            dfs_queue.push(root);

            while(!q.empty()) {
                ChunkedLODTerrainNode* node = q.front();
                q.pop();        

                if(ShouldSplitNode(node, camera.pos)) {
                    if(!node->children) {                
                        float child_size = node->size / 2.f;
                        float half_child_size = child_size / 2.f;
                        double child_pos[4][2] = {
                            { node->x - half_child_size, node->y + half_child_size },
                            { node->x + half_child_size, node->y + half_child_size },
                            { node->x - half_child_size, node->y - half_child_size },
                            { node->x + half_child_size, node->y - half_child_size },
                        };               x

                        node->children = new ChunkedLODTerrainNode[k_num_quad_tree_nodes];
                        for(uint32_t i = 0; i < k_num_quad_tree_nodes; ++i) {
                            ChunkedLODTerrainNode* child = &node->children[i];
                                                      
                            child->x = child_pos[i][0];
                            child->y = child_pos[i][1];
                            child->size = child_size;
                            child->lod = node->lod + 1;
                            child->tx = node->tx * 2 + (i % 2 == 0 ? 0 : 1);
                            child->ty = node->ty * 2 + (i < 2 ? 1 : 0);
                            child->bbox.max = glm::vec3(child->x + half_child_size, child->y + half_child_size, 0);
                            child->bbox.min = glm::vec3(child->x - half_child_size, child->y - half_child_size, 0);
                        }
                    }

                    for(uint32_t idx = 0; idx < k_num_quad_tree_nodes; ++idx) {
                        q.push(&node->children[idx]);
                    }   
                } else {

                }

            }
        }
    }
private:
    bool ShouldSplitNode(const ChunkedLODTerrainNode* node, const glm::vec3& eye) {
        if(node->lod >= k_max_lod) {
            return false;
        }

        return ComputeScreenSpaceError(node->bbox, eye, node->size);
    }

    bool ComputeScreenSpaceError(const BoundingBox& bbox, const glm::vec3& eye, float size) {
        double d = bbox.GetDistanceFromBoundingBox(eye);            
        if(d < k_split_factor * size) {
            return true;
        }

        return false;
    }
}

#endif