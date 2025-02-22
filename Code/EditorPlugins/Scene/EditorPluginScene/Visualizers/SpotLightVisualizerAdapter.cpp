#include <EditorPluginScene/EditorPluginScenePCH.h>

#include <EditorFramework/Assets/AssetDocument.h>
#include <EditorPluginScene/Visualizers/SpotLightVisualizerAdapter.h>
#include <RendererCore/Lights/SpotLightComponent.h>
#include <ToolsFoundation/Object/ObjectAccessorBase.h>

ezSpotLightVisualizerAdapter::ezSpotLightVisualizerAdapter() = default;

ezSpotLightVisualizerAdapter::~ezSpotLightVisualizerAdapter() = default;

void ezSpotLightVisualizerAdapter::Finalize()
{
  auto* pDoc = m_pObject->GetDocumentObjectManager()->GetDocument()->GetMainDocument();
  const ezAssetDocument* pAssetDocument = ezDynamicCast<const ezAssetDocument*>(pDoc);
  EZ_ASSERT_DEV(pAssetDocument != nullptr, "Visualizers are only supported in ezAssetDocument.");

  m_hGizmo.ConfigureHandle(nullptr, ezEngineGizmoHandleType::Cone, ezColor::White, ezGizmoFlags::ShowInOrtho | ezGizmoFlags::Visualizer);

  pAssetDocument->AddSyncObject(&m_hGizmo);
  m_hGizmo.SetVisible(m_bVisualizerIsVisible);
}

void ezSpotLightVisualizerAdapter::Update()
{
  const ezSpotLightVisualizerAttribute* pAttr = static_cast<const ezSpotLightVisualizerAttribute*>(m_pVisualizerAttr);
  ezObjectAccessorBase* pObjectAccessor = GetObjectAccessor();
  m_hGizmo.SetVisible(m_bVisualizerIsVisible);

  m_fAngleScale = 1.0f;
  if (!pAttr->GetAngleProperty().IsEmpty())
  {
    ezVariant value;
    pObjectAccessor->GetValue(m_pObject, GetProperty(pAttr->GetAngleProperty()), value).AssertSuccess();

    EZ_ASSERT_DEBUG(value.IsValid() && value.CanConvertTo<ezAngle>(), "Invalid property bound to ezSpotLightVisualizerAttribute 'angle'");
    m_fAngleScale = ezMath::Tan(value.ConvertTo<ezAngle>() * 0.5f);
  }

  if (!pAttr->GetColorProperty().IsEmpty())
  {
    ezVariant value;
    pObjectAccessor->GetValue(m_pObject, GetProperty(pAttr->GetColorProperty()), value).AssertSuccess();

    EZ_ASSERT_DEBUG(value.IsValid() && value.CanConvertTo<ezColor>(), "Invalid property bound to ezSpotLightVisualizerAttribute 'color'");
    m_hGizmo.SetColor(value.ConvertTo<ezColor>());
  }

  m_fScale = 1.0f;
  if (!pAttr->GetRangeProperty().IsEmpty() && !pAttr->GetIntensityProperty().IsEmpty())
  {
    ezVariant range;
    pObjectAccessor->GetValue(m_pObject, GetProperty(pAttr->GetRangeProperty()), range).AssertSuccess();
    EZ_ASSERT_DEBUG(range.CanConvertTo<float>(), "Invalid property bound to ezPointLightVisualizerAttribute 'radius'");

    ezVariant intensity;
    pObjectAccessor->GetValue(m_pObject, GetProperty(pAttr->GetIntensityProperty()), intensity).AssertSuccess();
    EZ_ASSERT_DEBUG(intensity.CanConvertTo<float>(), "Invalid property bound to ezPointLightVisualizerAttribute 'intensity'");

    m_fScale = ezLightComponent::CalculateEffectiveRange(range.ConvertTo<float>(), intensity.ConvertTo<float>());
  }

  m_hGizmo.SetVisible(m_fAngleScale != 0.0f && m_fScale != 0.0f);
}

void ezSpotLightVisualizerAdapter::UpdateGizmoTransform()
{
  ezTransform t = GetObjectTransform();
  t.m_vScale = t.m_vScale.CompMul(ezVec3(1.0f, m_fAngleScale, m_fAngleScale) * m_fScale);
  m_hGizmo.SetTransformation(t);
}
