#pragma once

#include <PhysXPlugin/Components/PxActorComponent.h>
#include <PhysXPlugin/Resources/PxMeshResource.h>

using ezPxCenterOfMassComponentManager = ezComponentManager<class ezPxCenterOfMassComponent, ezBlockStorageType::Compact>;

class EZ_PHYSXPLUGIN_DLL ezPxCenterOfMassComponent : public ezPxComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezPxCenterOfMassComponent, ezPxComponent, ezPxCenterOfMassComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // ezPxCenterOfMassComponent

public:
  ezPxCenterOfMassComponent();
  ~ezPxCenterOfMassComponent();
};
