#pragma once

#include <memory>
#include <vector>

namespace gfx {

enum class DispatchItemField : uint8_t { BindingCount = 0, DispatchCall, PipelineState, Bindings};

struct DispatchItem : public std::vector<uint8_t> {};
using DispatchItemPtr = std::shared_ptr<const DispatchItem>;
}
