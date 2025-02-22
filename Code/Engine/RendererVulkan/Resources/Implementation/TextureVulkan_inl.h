vk::Image ezGALTextureVulkan::GetImage() const
{
  return m_image;
}

vk::ImageLayout ezGALTextureVulkan::GetPreferredLayout() const
{
  return m_preferredLayout;
}

vk::ImageLayout ezGALTextureVulkan::GetPreferredLayout(vk::ImageLayout targetLayout) const
{
  return targetLayout;
  // #TODO_VULKAN Maintaining UAVs in general layout causes verification failures. For now, switch back and forth between layouts.
  // return m_preferredLayout == vk::ImageLayout::eGeneral ? vk::ImageLayout::eGeneral : targetLayout;
}

vk::PipelineStageFlags ezGALTextureVulkan::GetUsedByPipelineStage() const
{
  return m_stages;
}

vk::AccessFlags ezGALTextureVulkan::GetAccessMask() const
{
  return m_access;
}

ezVulkanAllocation ezGALTextureVulkan::GetAllocation() const
{
  return m_alloc;
}

const ezVulkanAllocationInfo& ezGALTextureVulkan::GetAllocationInfo() const
{
  return m_allocInfo;
}
