#include <EnginePluginAssets/EnginePluginAssetsPCH.h>

#include <EnginePluginAssets/TextureAsset/TextureContext.h>
#include <EnginePluginAssets/TextureAsset/TextureView.h>

#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererFoundation/RendererReflection.h>

ezTextureViewContext::ezTextureViewContext(ezTextureContext* pContext)
  : ezEngineProcessViewContext(pContext)
{
  m_pTextureContext = pContext;
}

ezTextureViewContext::~ezTextureViewContext() = default;

ezViewHandle ezTextureViewContext::CreateView()
{
  ezView* pView = nullptr;
  ezRenderWorld::CreateView("Texture Editor - View", pView);
  pView->SetCameraUsageHint(ezCameraUsageHint::EditorView);

  pView->SetRenderPipelineResource(CreateDebugRenderPipeline());
  pView->SetRenderPassProperty("DepthPrePass", "Active", false);
  pView->SetRenderPassProperty("AOPass", "Active", false);

  ezEngineProcessDocumentContext* pDocumentContext = GetDocumentContext();
  pView->SetWorld(pDocumentContext->GetWorld());
  pView->SetCamera(&m_Camera);
  return pView->GetHandle();
}

void ezTextureViewContext::SetCamera(const ezViewRedrawMsgToEngine* pMsg)
{
  // Do not apply render mode here otherwise we would switch to a different pipeline.
  // Also use hard-coded clipping planes so the quad is not culled too early.

  ezCameraMode::Enum cameraMode = (ezCameraMode::Enum)pMsg->m_iCameraMode;
  m_Camera.SetCameraMode(cameraMode, pMsg->m_fFovOrDim, 0.0001f, 50.0f);
  m_Camera.LookAt(pMsg->m_vPosition, pMsg->m_vPosition + pMsg->m_vDirForwards, pMsg->m_vDirUp);

  // Draw some stats
  auto hResource = m_pTextureContext->GetTexture();
  if (hResource.IsValid())
  {
    ezResourceLock<ezTexture2DResource> pResource(hResource, ezResourceAcquireMode::AllowLoadingFallback);
    const ezGALResourceFormat::Enum format = pResource->GetFormat();
    const int iMipLevel = m_pTextureContext->GetLodLevel();
    const ezUInt32 uiWidth = pResource->GetWidth();
    const ezUInt32 uiHeight = pResource->GetHeight();

    ezStringBuilder sText;
    if (!ezReflectionUtils::EnumerationToString(ezGetStaticRTTI<ezGALResourceFormat>(), format, sText, ezReflectionUtils::EnumConversionMode::ValueNameOnly))
    {
      sText = "Unknown format";
    }

    sText.PrependFormat("{}x{} - ", uiWidth, uiHeight);
    sText.Append("\nPreview Mip Level: ");

    if (iMipLevel < 0)
    {
      sText.Append("Auto");
    }
    else
    {
      const ezUInt32 uiMipWidth = ezMath::Max(1u, uiWidth >> ezMath::Max(iMipLevel, 0));
      const ezUInt32 uiMipHeight = ezMath::Max(1u, uiHeight >> ezMath::Max(iMipLevel, 0));

      sText.AppendFormat("{} ({}x{})", iMipLevel, uiMipWidth, uiMipHeight);
    }

    ezDebugRenderer::DrawInfoText(m_hView, ezDebugTextPlacement::BottomLeft, "AssetStats", sText);
  }
}
