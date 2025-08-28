#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

namespace RUBY
{
	class Instance;
	class IRubyWindow
	{
	public:
		virtual ~IRubyWindow() = default;

		virtual void PollEvents() = 0;
		virtual bool ShouldClose() const = 0;

		virtual int GetWidth() const = 0;
		virtual int GetHeight() const = 0;

		virtual bool IsResized() const = 0;
		virtual void SetResized() const = 0;

		virtual void WaitForEvents() const = 0;
		virtual void GetFramebufferSize(int* width, int* height) const = 0;

		virtual std::vector<const char*> GetRequiredInstanceExtensions() const = 0;
		virtual void CreateVkSurface(Instance& instance, VkSurfaceKHR* vkSurface) const = 0;
	};
}
