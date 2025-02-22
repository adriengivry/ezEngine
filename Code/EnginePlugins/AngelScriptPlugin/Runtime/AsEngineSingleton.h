#pragma once

#include <AngelScriptPlugin/AngelScriptPluginDLL.h>
#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Memory/CommonAllocators.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/UniquePtr.h>

class asIScriptEngine;
class asIScriptModule;
class asIStringFactory;
struct asSMessageInfo;
class ezAsStringFactory;

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
// AngelScript will allocate thread-local data, for which there currently is no way to free it again, so we don't want to report memory leaks
using ezAsAllocatorType = ezAllocatorWithPolicy<ezAllocPolicyHeap, ezAllocatorTrackingMode::AllocationStatsIgnoreLeaks>;
#else
using ezAsAllocatorType = ezAllocatorWithPolicy<ezAllocPolicyHeap, ezAllocatorTrackingMode::Nothing>;
#endif

class EZ_ANGELSCRIPTPLUGIN_DLL ezAngelScriptEngineSingleton
{
  EZ_DECLARE_SINGLETON(ezAngelScriptEngineSingleton);

public:
  ezAngelScriptEngineSingleton();
  ~ezAngelScriptEngineSingleton();

  asIScriptEngine* GetEngine() const { return m_pEngine; }

  asIScriptModule* SetModuleCode(ezStringView sModuleName, ezStringView sCode, bool bAddExternalSection);
  asIScriptModule* CompileModule(ezStringView sModuleName, ezStringView sMainClass, ezStringView sRefFilePath, ezStringView sCode, ezStringBuilder* out_pProcessedCode);
  ezResult ValidateModule(asIScriptModule* pModule) const;

  const ezSet<ezString>& GetNotRegistered() const { return m_NotRegistered; }

private:
  void AddForbiddenType(const char* szTypeName);
  bool IsTypeForbidden(const asITypeInfo* pType) const;
  static void MessageCallback(const asSMessageInfo* msg, void* param);
  void ExceptionCallback(asIScriptContext* pContext);

  void RegisterStandardTypes();
  void Register_RTTI();
  void Register_Vec2();
  void Register_Vec3();
  void Register_Vec4();
  void Register_Angle();
  void Register_Quat();
  void Register_Transform();
  void Register_GameObject();
  void Register_Time();
  void Register_Mat3();
  void Register_Mat4();
  void Register_World();
  void Register_Clock();
  void Register_String();
  void Register_StringView();
  void Register_StringBuilder();
  void Register_TempHashedString();
  void Register_HashedString();
  void Register_Color();
  void Register_ColorGammaUB();
  void Register_Random();
  void Register_Math();
  void Register_Spatial();

  void Register_ezAngelScriptClass();
  void Register_GlobalReflectedFunctions();
  void Register_ReflectedType(const ezRTTI* pBaseType, bool bCreatable);
  void Register_ReflectedTypes();
  void RegisterTypeFunctions(const char* szTypeName, const ezRTTI* pRtti, bool bIsInherited);
  void RegisterTypeProperties(const char* szTypeName, const ezRTTI* pRtti, bool bIsInherited);
  void RegisterGenericFunction(const char* szTypeName, const ezAbstractFunctionProperty* const pFunc, const ezScriptableFunctionAttribute* pFuncAttr, bool bIsInherited);
  bool AppendType(ezStringBuilder& decl, const ezRTTI* pRtti, const ezScriptableFunctionAttribute* pFuncAttr, ezUInt32 uiArg, bool& inout_VarArgs);
  bool AppendFuncArgs(ezStringBuilder& decl, const ezAbstractFunctionProperty* pFunc, const ezScriptableFunctionAttribute* pFuncAttr, ezUInt32 uiArg, bool& inout_VarArgs);
  ezString Register_EnumType(const ezRTTI* pEnumType);
  void Register_ExtraComponentFuncs();


  template <typename T>
  void RegisterPodValueType()
  {
    const ezRTTI* pRtti = ezGetStaticRTTI<T>();
    int typeId = m_pEngine->RegisterObjectType(pRtti->GetTypeName().GetStartPointer(), sizeof(T), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<T>());
    AS_CHECK(typeId);

    m_pEngine->GetTypeInfoById(typeId)->SetUserData((void*)pRtti, ezAsUserData::RttiPtr);
  }

  template <typename T>
  void RegisterNonPodValueType()
  {
    const ezRTTI* pRtti = ezGetStaticRTTI<T>();
    int typeId = m_pEngine->RegisterObjectType(pRtti->GetTypeName().GetStartPointer(), sizeof(T), asOBJ_VALUE | asGetTypeTraits<T>());
    AS_CHECK(typeId);

    m_pEngine->GetTypeInfoById(typeId)->SetUserData((void*)pRtti, ezAsUserData::RttiPtr);
  }

  template <typename T>
  void RegisterRefType()
  {
    const ezRTTI* pRtti = ezGetStaticRTTI<T>();
    int typeId = m_pEngine->RegisterObjectType(pRtti->GetTypeName().GetStartPointer(), 0, asOBJ_REF | asOBJ_NOCOUNT);
    AS_CHECK(typeId);

    m_pEngine->GetTypeInfoById(typeId)->SetUserData((void*)pRtti, ezAsUserData::RttiPtr);

    AddForbiddenType(pRtti->GetTypeName().GetStartPointer());

    m_WhitelistedRefTypes.Insert(pRtti->GetTypeName());
  }

  ezUniquePtr<ezAsAllocatorType> m_pAllocator;
  asIScriptEngine* m_pEngine = nullptr;

  ezSet<ezString> m_WhitelistedRefTypes;

  ezHybridArray<const asITypeInfo*, 16> m_ForbiddenTypes;

  ezAsStringFactory* m_pStringFactory = nullptr;

  ezSet<ezString> m_NotRegistered;
};
