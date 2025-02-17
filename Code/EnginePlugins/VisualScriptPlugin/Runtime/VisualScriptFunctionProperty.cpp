#include <VisualScriptPlugin/VisualScriptPluginPCH.h>

#include <Core/Scripting/ScriptComponent.h>
#include <Core/Scripting/ScriptWorldModule.h>
#include <VisualScriptPlugin/Runtime/VisualScriptFunctionProperty.h>
#include <VisualScriptPlugin/Runtime/VisualScriptInstance.h>

ezVisualScriptFunctionProperty::ezVisualScriptFunctionProperty(ezStringView sName, const ezSharedPtr<const ezVisualScriptGraphDescription>& pDesc)
  : ezScriptFunctionProperty(sName)
  , m_pDesc(pDesc)
{
  EZ_ASSERT_DEBUG(m_pDesc->IsCoroutine() == false, "Must not be a coroutine");
}

ezVisualScriptFunctionProperty::~ezVisualScriptFunctionProperty() = default;

void ezVisualScriptFunctionProperty::Execute(void* pInstance, ezArrayPtr<ezVariant> arguments, ezVariant& out_returnValue) const
{
  EZ_ASSERT_DEBUG(pInstance != nullptr, "Invalid instance");
  auto pVisualScriptInstance = static_cast<ezVisualScriptInstance*>(pInstance);

  ezVisualScriptExecutionContext context(m_pDesc, ezFrameAllocator::GetCurrentAllocator());
  context.Initialize(*pVisualScriptInstance, arguments);

  auto result = context.Execute(ezTime::MakeZero());
  EZ_ASSERT_DEBUG(result.m_NextExecAndState != ezVisualScriptExecutionContext::ExecResult::State::ContinueLater, "A non-coroutine function must not return 'ContinueLater'");

  // TODO: return value
}

//////////////////////////////////////////////////////////////////////////

ezVisualScriptMessageHandler::ezVisualScriptMessageHandler(const ezScriptMessageDesc& desc, const ezSharedPtr<const ezVisualScriptGraphDescription>& pDesc)
  : ezScriptMessageHandler(desc)
  , m_pDesc(pDesc)
{
  EZ_ASSERT_DEBUG(m_pDesc->IsCoroutine() == false, "Must not be a coroutine");

  m_DispatchFunc = &Dispatch;
}

ezVisualScriptMessageHandler::~ezVisualScriptMessageHandler() = default;

// static
void ezVisualScriptMessageHandler::Dispatch(ezAbstractMessageHandler* pSelf, void* pInstance, ezMessage& ref_msg)
{
  auto pHandler = static_cast<ezVisualScriptMessageHandler*>(pSelf);
  auto pComponent = static_cast<ezScriptComponent*>(pInstance);
  auto pVisualScriptInstance = static_cast<ezVisualScriptInstance*>(pComponent->GetScriptInstance());

  ezHybridArray<ezVariant, 8> arguments;
  pHandler->FillMessagePropertyValues(ref_msg, arguments);

  ezVisualScriptExecutionContext context(pHandler->m_pDesc, ezFrameAllocator::GetCurrentAllocator());
  context.Initialize(*pVisualScriptInstance, arguments);

  auto result = context.Execute(ezTime::MakeZero());
  EZ_ASSERT_DEBUG(result.m_NextExecAndState != ezVisualScriptExecutionContext::ExecResult::State::ContinueLater, "A non-coroutine function must not return 'ContinueLater'");
}
