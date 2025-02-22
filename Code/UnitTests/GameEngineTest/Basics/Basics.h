#pragma once

#include <GameEngineTest/GameEngineTestPCH.h>

#include "../TestClass/TestClass.h"

class ezGameEngineTestApplication_Basics : public ezGameEngineTestApplication
{
public:
  ezGameEngineTestApplication_Basics();

  void SubTestManyMeshesSetup();
  ezTestAppRun SubTestManyMeshesExec(ezInt32 iCurFrame);

  void SubTestSkyboxSetup();
  ezTestAppRun SubTestSkyboxExec(ezInt32 iCurFrame);

  void SubTestDebugRenderingSetup();
  ezTestAppRun SubTestDebugRenderingExec(ezInt32 iCurFrame);

  ezTestAppRun SubTestDebugRenderingExec2(ezInt32 iCurFrame);

  void SubTestLoadSceneSetup();
  ezTestAppRun SubTestLoadSceneExec(ezInt32 iCurFrame);

  void SubTestGoReferenceSetup();
  ezTestAppRun SubTestGoReferenceExec(ezInt32 iCurFrame);
};

class ezGameEngineTestBasics : public ezGameEngineTest
{
  using SUPER = ezGameEngineTest;

public:
  virtual const char* GetTestName() const override;
  virtual ezGameEngineTestApplication* CreateApplication() override;

private:
  enum SubTests
  {
    ManyMeshes,
    Skybox,
    DebugRendering,
    DebugRendering2,
    LoadScene,
    GameObjectReferences,
  };

  virtual void SetupSubTests() override;
  virtual ezResult InitializeSubTest(ezInt32 iIdentifier) override;
  virtual ezTestAppRun RunSubTest(ezInt32 iIdentifier, ezUInt32 uiInvocationCount) override;

  ezInt32 m_iFrame;
  ezGameEngineTestApplication_Basics* m_pOwnApplication;
};
