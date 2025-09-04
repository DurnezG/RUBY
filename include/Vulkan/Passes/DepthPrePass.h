#pragma once

#include "IBasePass.h"

//namespace RUBY
//{
//	class DepthPrePass final : IBasePass
//	{
//	public:
//		~DepthPrePass() override;
//
//		DepthPrePass(const DepthPrePass& other) = delete;
//		DepthPrePass(DepthPrePass&& other) noexcept = delete;
//		DepthPrePass& operator=(const DepthPrePass& other) = delete;
//		DepthPrePass& operator=(DepthPrePass&& other) noexcept = delete;
//
//		void CreateDescriptorSets() override;
//		void Update(uint32_t imageIndex, IScene* pScene);
//		void OnResize() override;
//
//		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, PassContext& passContext) override;
//	};
//}