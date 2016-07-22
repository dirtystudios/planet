#pragma once

#include "CommandBuffer.h"
#include <vector>
#include "Bytebuffer.h"

namespace gfx {

	class DX11CommandBuffer : public CommandBuffer {
	public:
		enum class CommandType {
			Clear,
			DrawItem,
			BindResource
		};

	private:
		static constexpr int kBufferSize = 4096;
		ByteBuffer _byteBuffer;
	public:
		DX11CommandBuffer() {
			_byteBuffer.Resize(kBufferSize);
			Reset();
		}
		~DX11CommandBuffer() {
		}

		void Clear(float r, float g, float b, float a) {
			_byteBuffer << CommandType::Clear;
		}

		void DrawItem(const struct DrawItem* drawItem) {
			_byteBuffer << CommandType::DrawItem << drawItem;
		}

		void BindResource(const Binding& binding) {
			_byteBuffer << CommandType::BindResource << binding;
		}

		// TODO:: readonly interface for bytebuffer?
		ByteBuffer& GetByteBuffer() {
			return _byteBuffer;
		}

		void Reset() {
			_byteBuffer.Reset();
		}
	};
}

