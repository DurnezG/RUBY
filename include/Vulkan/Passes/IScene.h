#pragma once
#include <vector>
#include <Vulkan/Buffer.h>

namespace RUBY
{
	class IScene
	{
	public:
		virtual std::vector<Buffer*> GetVertexBuffers() = 0;
		virtual UniformBufferObject* GetUniformBuffer() = 0;
	};
}