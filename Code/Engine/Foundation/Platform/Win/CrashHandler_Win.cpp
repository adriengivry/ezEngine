#include <Foundation/FoundationPCH.h>

#if EZ_ENABLED(EZ_PLATFORM_WINDOWS)

#  include <Foundation/Logging/Log.h>
#  include <Foundation/System/CrashHandler.h>
#  include <Foundation/System/MiniDumpUtils.h>
#  include <Foundation/System/StackTracer.h>

static void PrintHelper(const char* szString)
{
  ezLog::Printf("%s", szString);
}

static LONG WINAPI ezCrashHandlerFunc(struct _EXCEPTION_POINTERS* pExceptionInfo)
{
  static ezMutex s_CrashMutex;
  EZ_LOCK(s_CrashMutex);

  static bool s_bAlreadyHandled = false;

  if (s_bAlreadyHandled == false)
  {
    if (ezCrashHandler::GetCrashHandler() != nullptr)
    {
      s_bAlreadyHandled = true;
      ezCrashHandler::GetCrashHandler()->HandleCrash(pExceptionInfo);
    }
  }

  return EXCEPTION_CONTINUE_SEARCH;
}

void ezCrashHandler::SetCrashHandler(ezCrashHandler* pHandler)
{
  s_pActiveHandler = pHandler;

  if (s_pActiveHandler != nullptr)
  {
    SetUnhandledExceptionFilter(ezCrashHandlerFunc);
  }
  else
  {
    SetUnhandledExceptionFilter(nullptr);
  }
}

bool ezCrashHandler_WriteMiniDump::WriteOwnProcessMiniDump(void* pOsSpecificData)
{
#  if EZ_ENABLED(EZ_PLATFORM_WINDOWS_DESKTOP)
  ezStatus res = ezMiniDumpUtils::WriteOwnProcessMiniDump(m_sDumpFilePath, (_EXCEPTION_POINTERS*)pOsSpecificData);
  if (res.Failed())
    ezLog::Printf("WriteOwnProcessMiniDump failed: %s\n", res.m_sMessage.GetData());
  return res.Succeeded();
#  else
  EZ_IGNORE_UNUSED(pOsSpecificData);
  return false;
#  endif
}

void ezCrashHandler_WriteMiniDump::PrintStackTrace(void* pOsSpecificData)
{
  _EXCEPTION_POINTERS* pExceptionInfo = (_EXCEPTION_POINTERS*)pOsSpecificData;

  ezLog::Printf("***Unhandled Exception:***\n");
  ezLog::Printf("Exception: %08x", (ezUInt32)pExceptionInfo->ExceptionRecord->ExceptionCode);

  {
    ezLog::Printf("\n\n***Stack Trace:***\n");
    void* pBuffer[64];
    ezArrayPtr<void*> tempTrace(pBuffer);
    const ezUInt32 uiNumTraces = ezStackTracer::GetStackTrace(tempTrace, pExceptionInfo->ContextRecord);

    ezStackTracer::ResolveStackTrace(tempTrace.GetSubArray(0, uiNumTraces), &PrintHelper);
  }
}

#endif
