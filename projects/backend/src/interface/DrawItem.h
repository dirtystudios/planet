#pragma once

#include <memory>
#include <vector>

namespace gfx {

enum class DrawItemField : uint8_t { StreamCount = 0, BindingCount, DrawCall, IndexBuffer, PipelineState, VertexStreams, Bindings };

struct DrawItem : public std::vector<uint8_t> {};
using DrawItemPtr = std::shared_ptr<const DrawItem>;
}
