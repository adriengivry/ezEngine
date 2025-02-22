#include <EditorFramework/EditorFrameworkPCH.h>

#include <EditorFramework/Preferences/QuadViewPreferences.h>
#include <Foundation/Serialization/GraphPatch.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_TYPE(ezEngineViewPreferences, ezNoBase, 2, ezRTTIDefaultAllocator<ezEngineViewPreferences>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("CamPos", m_vCamPos),
    EZ_MEMBER_PROPERTY("CamDir", m_vCamDir),
    EZ_MEMBER_PROPERTY("CamUp", m_vCamUp),
    EZ_ENUM_MEMBER_PROPERTY("Perspective", ezSceneViewPerspective, m_PerspectiveMode),
    EZ_ENUM_MEMBER_PROPERTY("RenderMode", ezViewRenderMode, m_RenderMode),
    EZ_MEMBER_PROPERTY("FOV", m_fFov),
  }
  EZ_END_PROPERTIES;
}
EZ_END_STATIC_REFLECTED_TYPE;
// clang-format on

namespace
{
  /// Patch class
  class ezSceneViewPreferencesPatch_1_2 : public ezGraphPatch
  {
  public:
    ezSceneViewPreferencesPatch_1_2()
      : ezGraphPatch("ezSceneViewPreferences", 2)
    {
    }
    virtual void Patch(ezGraphPatchContext& ref_context, ezAbstractObjectGraph* pGraph, ezAbstractObjectNode* pNode) const override
    {
      ref_context.RenameClass("ezEngineViewPreferences");
    }
  };
  ezSceneViewPreferencesPatch_1_2 g_ezSceneViewPreferencesPatch_1_2;
} // namespace

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezQuadViewPreferencesUser, 1, ezRTTIDefaultAllocator<ezQuadViewPreferencesUser>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("QuadView", m_bQuadView)->AddAttributes(new ezHiddenAttribute()),
    EZ_MEMBER_PROPERTY("ViewSingle", m_ViewSingle)->AddAttributes(new ezHiddenAttribute()),
    EZ_MEMBER_PROPERTY("ViewQuad0", m_ViewQuad0)->AddAttributes(new ezHiddenAttribute()),
    EZ_MEMBER_PROPERTY("ViewQuad1", m_ViewQuad1)->AddAttributes(new ezHiddenAttribute()),
    EZ_MEMBER_PROPERTY("ViewQuad2", m_ViewQuad2)->AddAttributes(new ezHiddenAttribute()),
    EZ_MEMBER_PROPERTY("ViewQuad3", m_ViewQuad3)->AddAttributes(new ezHiddenAttribute()),
    EZ_ARRAY_ACCESSOR_PROPERTY("FavoriteCams", FavCams_GetCount, FavCams_GetCam, FavCams_SetCam, FavCams_Insert, FavCams_Remove)->AddAttributes(new ezHiddenAttribute()),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezQuadViewPreferencesUser::ezQuadViewPreferencesUser()
  : ezPreferences(Domain::Document, "View")
{
  m_bQuadView = false;

  m_ViewSingle.m_vCamPos.Set(3, 0.5f, 1);
  m_ViewSingle.m_vCamDir = -m_ViewSingle.m_vCamPos.GetNormalized();
  ezVec3 vRight = m_ViewSingle.m_vCamDir.CrossRH(ezVec3(0, 0, 1));
  m_ViewSingle.m_vCamUp = vRight.CrossRH(m_ViewSingle.m_vCamDir).GetNormalized();
  m_ViewSingle.m_PerspectiveMode = ezSceneViewPerspective::Perspective;
  m_ViewSingle.m_RenderMode = ezViewRenderMode::Default;
  m_ViewSingle.m_fFov = 70.0f;

  // Top Left: Top Down
  m_ViewQuad0.m_vCamPos.SetZero();
  m_ViewQuad0.m_vCamDir.Set(0, 0, -1);
  m_ViewQuad0.m_vCamUp.Set(1, 0, 0);
  m_ViewQuad0.m_PerspectiveMode = ezSceneViewPerspective::Orthogonal_Top;
  m_ViewQuad0.m_RenderMode = ezViewRenderMode::WireframeMonochrome;
  m_ViewQuad0.m_fFov = 20.0f;

  // Top Right: Perspective
  m_ViewQuad1.m_vCamPos = m_ViewSingle.m_vCamPos;
  m_ViewQuad1.m_vCamDir = m_ViewSingle.m_vCamDir;
  m_ViewQuad1.m_vCamUp = m_ViewSingle.m_vCamUp;
  m_ViewQuad1.m_PerspectiveMode = ezSceneViewPerspective::Perspective;
  m_ViewQuad1.m_RenderMode = ezViewRenderMode::Default;
  m_ViewQuad1.m_fFov = 70.0f;

  // Bottom Left: Front to Back
  m_ViewQuad2.m_vCamPos.SetZero();
  m_ViewQuad2.m_vCamDir.Set(-1, 0, 0);
  m_ViewQuad2.m_vCamUp.Set(0, 0, 1);
  m_ViewQuad2.m_PerspectiveMode = ezSceneViewPerspective::Orthogonal_Front;
  m_ViewQuad2.m_RenderMode = ezViewRenderMode::WireframeMonochrome;
  m_ViewQuad2.m_fFov = 20.0f;

  // Bottom Right: Right to Left
  m_ViewQuad3.m_vCamPos.SetZero();
  m_ViewQuad3.m_vCamDir.Set(0, -1, 0);
  m_ViewQuad3.m_vCamUp.Set(0, 0, 1);
  m_ViewQuad3.m_PerspectiveMode = ezSceneViewPerspective::Orthogonal_Right;
  m_ViewQuad3.m_RenderMode = ezViewRenderMode::WireframeMonochrome;
  m_ViewQuad3.m_fFov = 20.0f;
}
