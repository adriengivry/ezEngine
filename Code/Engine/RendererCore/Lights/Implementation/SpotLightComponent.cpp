#include <RendererCore/RendererCorePCH.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Lights/Implementation/ShadowPool.h>
#include <RendererCore/Lights/SpotLightComponent.h>
#include <RendererCore/Pipeline/View.h>

constexpr ezAngle c_MaxSpotAngle = ezAngle::MakeFromDegree(160.0f);

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezSpotLightRenderData, 1, ezRTTIDefaultAllocator<ezSpotLightRenderData>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_COMPONENT_TYPE(ezSpotLightComponent, 3, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ACCESSOR_PROPERTY("Range", GetRange, SetRange)->AddAttributes(new ezClampValueAttribute(0.0f, ezVariant()), new ezDefaultValueAttribute(0.0f), new ezSuffixAttribute(" m"), new ezMinValueTextAttribute("Auto")),
    EZ_ACCESSOR_PROPERTY("InnerSpotAngle", GetInnerSpotAngle, SetInnerSpotAngle)->AddAttributes(new ezClampValueAttribute(ezAngle::MakeZero(), c_MaxSpotAngle), new ezDefaultValueAttribute(ezAngle::MakeFromDegree(15.0f))),
    EZ_ACCESSOR_PROPERTY("OuterSpotAngle", GetOuterSpotAngle, SetOuterSpotAngle)->AddAttributes(new ezClampValueAttribute(ezAngle::MakeZero(), c_MaxSpotAngle), new ezDefaultValueAttribute(ezAngle::MakeFromDegree(30.0f))),
    EZ_ACCESSOR_PROPERTY("ShadowFadeOutRange", GetShadowFadeOutRange, SetShadowFadeOutRange)->AddAttributes(new ezClampValueAttribute(0.0f, ezVariant()), new ezSuffixAttribute(" m"), new ezMinValueTextAttribute("Auto")),
    //EZ_ACCESSOR_PROPERTY("ProjectedTexture", GetProjectedTextureFile, SetProjectedTextureFile)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Texture_2D")),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_MESSAGEHANDLERS
  {
    EZ_MESSAGE_HANDLER(ezMsgExtractRenderData, OnMsgExtractRenderData),
  }
  EZ_END_MESSAGEHANDLERS;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezSpotLightVisualizerAttribute("OuterSpotAngle", "Range", "Intensity", "LightColor"),
    new ezConeLengthManipulatorAttribute("Range"),
    new ezConeAngleManipulatorAttribute("OuterSpotAngle", 1.5f),
    new ezConeAngleManipulatorAttribute("InnerSpotAngle", 1.5f),
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_COMPONENT_TYPE
// clang-format on

ezSpotLightComponent::ezSpotLightComponent() = default;
ezSpotLightComponent::~ezSpotLightComponent() = default;

ezResult ezSpotLightComponent::GetLocalBounds(ezBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, ezMsgUpdateLocalBounds& ref_msg)
{
  m_fEffectiveRange = CalculateEffectiveRange(m_fRange, m_fIntensity);

  ref_bounds = CalculateBoundingSphere(ezTransform::MakeIdentity(), m_fEffectiveRange);
  return EZ_SUCCESS;
}

void ezSpotLightComponent::SetRange(float fRange)
{
  m_fRange = fRange;

  TriggerLocalBoundsUpdate();
}

float ezSpotLightComponent::GetRange() const
{
  return m_fRange;
}

float ezSpotLightComponent::GetEffectiveRange() const
{
  return m_fEffectiveRange;
}

void ezSpotLightComponent::SetShadowFadeOutRange(float fRange)
{
  m_fShadowFadeOutRange = ezMath::Max(fRange, 0.0f);

  InvalidateCachedRenderData();
}

float ezSpotLightComponent::GetShadowFadeOutRange() const
{
  return m_fShadowFadeOutRange;
}

void ezSpotLightComponent::SetInnerSpotAngle(ezAngle spotAngle)
{
  m_InnerSpotAngle = ezMath::Clamp(spotAngle, ezAngle::MakeZero(), m_OuterSpotAngle);

  InvalidateCachedRenderData();
}

ezAngle ezSpotLightComponent::GetInnerSpotAngle() const
{
  return m_InnerSpotAngle;
}

void ezSpotLightComponent::SetOuterSpotAngle(ezAngle spotAngle)
{
  m_OuterSpotAngle = ezMath::Clamp(spotAngle, m_InnerSpotAngle, c_MaxSpotAngle);

  TriggerLocalBoundsUpdate();
}

ezAngle ezSpotLightComponent::GetOuterSpotAngle() const
{
  return m_OuterSpotAngle;
}

// void ezSpotLightComponent::SetProjectedTexture(const ezTexture2DResourceHandle& hProjectedTexture)
//{
//   m_hProjectedTexture = hProjectedTexture;
//
//   InvalidateCachedRenderData();
// }
//
// const ezTexture2DResourceHandle& ezSpotLightComponent::GetProjectedTexture() const
//{
//   return m_hProjectedTexture;
// }
//
// void ezSpotLightComponent::SetProjectedTextureFile(const char* szFile)
//{
//   ezTexture2DResourceHandle hProjectedTexture;
//
//   if (!ezStringUtils::IsNullOrEmpty(szFile))
//   {
//     hProjectedTexture = ezResourceManager::LoadResource<ezTexture2DResource>(szFile);
//   }
//
//   SetProjectedTexture(hProjectedTexture);
// }
//
// const char* ezSpotLightComponent::GetProjectedTextureFile() const
//{
//   if (!m_hProjectedTexture.IsValid())
//     return "";
//
//   return m_hProjectedTexture.GetResourceID();
// }

void ezSpotLightComponent::OnMsgExtractRenderData(ezMsgExtractRenderData& msg) const
{
  // Don't extract light render data for selection or in shadow views.
  if (msg.m_OverrideCategory != ezInvalidRenderDataCategory || msg.m_pView->GetCameraUsageHint() == ezCameraUsageHint::Shadow)
    return;

  if (m_fIntensity <= 0.0f || m_fEffectiveRange <= 0.0f || m_OuterSpotAngle.GetRadian() <= 0.0f)
    return;

  const ezTransform t = GetOwner()->GetGlobalTransform();
  ezBoundingSphere bounds = CalculateBoundingSphere(t, m_fEffectiveRange);
  bounds.m_vCenter = (bounds.m_vCenter + t.m_vPosition) * 0.5f; // Halfway between light origin and cone center

  const float fScreenSpaceSize = CalculateScreenSpaceSize(bounds, *msg.m_pView->GetCullingCamera());
  float fShadowScreenSize = 0.0f;
  const float fShadowFadeOut = CalculateShadowFadeOut(bounds, m_fShadowFadeOutRange, *msg.m_pView->GetCullingCamera(), fShadowScreenSize);

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
  VisualizeScreenSpaceSize(msg.m_pView->GetHandle(), bounds, fScreenSpaceSize, fShadowScreenSize, fShadowFadeOut);
#endif

  auto pRenderData = ezCreateRenderDataForThisFrame<ezSpotLightRenderData>(GetOwner());

  pRenderData->m_GlobalTransform = t;
  pRenderData->m_LightColor = GetEffectiveColor();
  pRenderData->m_fIntensity = m_fIntensity;
  pRenderData->m_fSpecularMultiplier = m_fSpecularMultiplier;
  pRenderData->m_fRange = m_fEffectiveRange;
  pRenderData->m_InnerSpotAngle = m_InnerSpotAngle;
  pRenderData->m_OuterSpotAngle = m_OuterSpotAngle;
  // pRenderData->m_hProjectedTexture = m_hProjectedTexture;

  if (m_bCastShadows && fShadowFadeOut > 0.0f)
  {
    pRenderData->FillShadowDataOffsetAndFadeOut(ezShadowPool::AddSpotLight(this, fScreenSpaceSize, msg.m_pView), fShadowFadeOut);
  }
  else
  {
    pRenderData->m_uiShadowDataOffsetAndFadeOut = 0;
  }

  pRenderData->FillBatchIdAndSortingKey(fScreenSpaceSize);

  ezRenderData::Caching::Enum caching = m_bCastShadows ? ezRenderData::Caching::Never : ezRenderData::Caching::IfStatic;
  msg.AddRenderData(pRenderData, ezDefaultRenderDataCategories::Light, caching);
}

void ezSpotLightComponent::SerializeComponent(ezWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);

  ezStreamWriter& s = inout_stream.GetStream();

  s << m_fRange;
  s << m_fShadowFadeOutRange;
  s << m_InnerSpotAngle;
  s << m_OuterSpotAngle;
  s << ""; // GetProjectedTextureFile();
}

void ezSpotLightComponent::DeserializeComponent(ezWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const ezUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());
  ezStreamReader& s = inout_stream.GetStream();

  ezTexture2DResourceHandle m_hProjectedTexture;

  s >> m_fRange;
  if (uiVersion >= 3)
  {
    s >> m_fShadowFadeOutRange;
  }
  s >> m_InnerSpotAngle;
  s >> m_OuterSpotAngle;

  ezStringBuilder temp;
  s >> temp;
  // SetProjectedTextureFile(temp);
}

ezBoundingSphere ezSpotLightComponent::CalculateBoundingSphere(const ezTransform& t, float fRange) const
{
  ezBoundingSphere res;
  ezAngle halfAngle = m_OuterSpotAngle / 2.0f;
  ezVec3 position = t.m_vPosition;
  ezVec3 forwardDir = t.m_qRotation * ezVec3(1.0f, 0.0f, 0.0f);

  if (halfAngle > ezAngle::MakeFromDegree(45.0f))
  {
    res.m_vCenter = position + ezMath::Cos(halfAngle) * fRange * forwardDir;
    res.m_fRadius = ezMath::Sin(halfAngle) * fRange;
  }
  else
  {
    res.m_fRadius = fRange / (2.0f * ezMath::Cos(halfAngle));
    res.m_vCenter = position + forwardDir * res.m_fRadius;
  }

  return res;
}

//////////////////////////////////////////////////////////////////////////

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezSpotLightVisualizerAttribute, 1, ezRTTIDefaultAllocator<ezSpotLightVisualizerAttribute>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

ezSpotLightVisualizerAttribute::ezSpotLightVisualizerAttribute()
  : ezVisualizerAttribute(nullptr)
{
}

ezSpotLightVisualizerAttribute::ezSpotLightVisualizerAttribute(
  const char* szAngleProperty, const char* szRangeProperty, const char* szIntensityProperty, const char* szColorProperty)
  : ezVisualizerAttribute(szAngleProperty, szRangeProperty, szIntensityProperty, szColorProperty)
{
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/GraphPatch.h>

class ezSpotLightComponentPatch_1_2 : public ezGraphPatch
{
public:
  ezSpotLightComponentPatch_1_2()
    : ezGraphPatch("ezSpotLightComponent", 2)
  {
  }

  virtual void Patch(ezGraphPatchContext& ref_context, ezAbstractObjectGraph* pGraph, ezAbstractObjectNode* pNode) const override
  {
    ref_context.PatchBaseClass("ezLightComponent", 2, true);

    pNode->RenameProperty("Inner Spot Angle", "InnerSpotAngle");
    pNode->RenameProperty("Outer Spot Angle", "OuterSpotAngle");
  }
};

ezSpotLightComponentPatch_1_2 g_ezSpotLightComponentPatch_1_2;


EZ_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_SpotLightComponent);
