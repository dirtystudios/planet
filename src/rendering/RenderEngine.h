#pragma once 

#include "Renderer.h"
#include "RendererType.h"
#include "RenderObj.h"
#include "RenderView.h"
#include "RenderDevice.h"
#include <unordered_map>
#include <cassert>

#include "ChunkedLoDTerrainRenderer.h"

using namespace graphics;

class RenderEngine {
private:
	RenderDevice* _device { nullptr };
	RenderView* _view { nullptr };
	std::unordered_map<RendererType, Renderer*> _renderers;
	std::unordered_map<RenderObj*, Renderer*> _renderObjLookup;
public:
	RenderEngine(RenderDevice* device, RenderView* view) 
	: _device(device), _view(view)
	{
		_renderers.insert( { RendererType::ChunkedTerrain, new ChunkedLoDTerrainRenderer(_device) } );
	};

	RenderObj* Register(SimObj* simObj, RendererType rendererType) {
		Renderer* renderer = GetRenderer(rendererType);
		assert(renderer);

		RenderObj* renderObj = renderer->Register(simObj);
		assert(renderObj);
		return renderObj;
	}

	void Unregister(RenderObj* renderObj) {
		Renderer* renderer = GetRenderer(renderObj->GetRendererType());
		assert(renderer);

		renderer->Unregister(renderObj);
	}

	void RenderFrame() {
		assert(_view);
		for(const std::pair<RendererType, Renderer*>& p : _renderers) {
			p.second->Submit(_view);
		}
	}

private:
	Renderer* GetRenderer(RendererType type) {
		auto it = _renderers.find(type);
		return (it == _renderers.end() ? nullptr : it->second);
	}
};



