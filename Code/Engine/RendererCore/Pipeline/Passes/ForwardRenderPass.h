#pragma once

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>

struct ezForwardRenderShadingQuality
{
  using StorageType = ezInt8;

  enum Enum
  {
    Normal,
    Simplified,

    Default = Normal,
  };
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_NO_LINKAGE, ezForwardRenderShadingQuality);

/// \brief A standard forward render pass that renders into the color target.
class EZ_RENDERERCORE_DLL ezForwardRenderPass : public ezRenderPipelinePass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezForwardRenderPass, ezRenderPipelinePass);

public:
  ezForwardRenderPass(const char* szName = "ForwardRenderPass");
  ~ezForwardRenderPass();

  virtual bool GetRenderTargetDescriptions(const ezView& view, const ezArrayPtr<ezGALTextureCreationDescription* const> inputs, ezArrayPtr<ezGALTextureCreationDescription> outputs) override;
  virtual void Execute(const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs) override;

  virtual ezResult Serialize(ezStreamWriter& inout_stream) const override;
  virtual ezResult Deserialize(ezStreamReader& inout_stream) override;

protected:
  virtual void SetupResources(ezGALCommandEncoder* pCommandEncoder, const ezRenderViewContext& renderViewContext, const ezArrayPtr<ezRenderPipelinePassConnection* const> inputs, const ezArrayPtr<ezRenderPipelinePassConnection* const> outputs);
  virtual void SetupPermutationVars(const ezRenderViewContext& renderViewContext);
  virtual void SetupLighting(const ezRenderViewContext& renderViewContext);

  virtual void RenderObjects(const ezRenderViewContext& renderViewContext) = 0;

  ezRenderPipelineNodePassThroughPin m_PinColor;
  ezRenderPipelineNodePassThroughPin m_PinDepthStencil;

  ezEnum<ezForwardRenderShadingQuality> m_ShadingQuality;
};
