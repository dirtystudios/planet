#pragma once

#include <glm/glm.hpp>

struct SkyboxVertex {
    glm::vec3 pos;

	static const graphics::VertexLayoutDesc& GetVertexLayoutDesc() {
		static graphics::VertexLayoutDesc layout {
		    {
		    	{
		     		graphics::VertexAttributeType::Float3, graphics::VertexAttributeUsage::Position,
				}
			}
		};

		return layout;
	}
};