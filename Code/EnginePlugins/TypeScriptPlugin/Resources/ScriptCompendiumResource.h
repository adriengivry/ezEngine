#pragma once

#include <TypeScriptPlugin/TypeScriptPluginDLL.h>

#include <Core/ResourceManager/Resource.h>
#include <Foundation/Containers/DynamicArray.h>

class ezStreamWriter;
class ezStreamReader;

struct EZ_TYPESCRIPTPLUGIN_DLL ezScriptCompendiumResourceDesc
{
  ezMap<ezString, ezString> m_PathToSource;

  struct ComponentTypeInfo
  {
    ezString m_sComponentTypeName;
    ezString m_sComponentFilePath;

    ezResult Serialize(ezStreamWriter& inout_stream) const;
    ezResult Deserialize(ezStreamReader& inout_stream);
  };

  ezMap<ezUuid, ComponentTypeInfo> m_AssetGuidToInfo;

  ezResult Serialize(ezStreamWriter& inout_stream) const;
  ezResult Deserialize(ezStreamReader& inout_stream);
};

class EZ_TYPESCRIPTPLUGIN_DLL ezScriptCompendiumResource : public ezResource
{
  EZ_ADD_DYNAMIC_REFLECTION(ezScriptCompendiumResource, ezResource);
  EZ_RESOURCE_DECLARE_COMMON_CODE(ezScriptCompendiumResource);

public:
  ezScriptCompendiumResource();
  ~ezScriptCompendiumResource();

  const ezScriptCompendiumResourceDesc& GetDescriptor() const { return m_Desc; }

private:
  virtual ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual ezResourceLoadDesc UpdateContent(ezStreamReader* pStream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

  ezScriptCompendiumResourceDesc m_Desc;
};

using ezScriptCompendiumResourceHandle = ezTypedResourceHandle<class ezScriptCompendiumResource>;
