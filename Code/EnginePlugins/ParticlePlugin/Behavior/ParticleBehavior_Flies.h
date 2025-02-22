#pragma once

#include <ParticlePlugin/Behavior/ParticleBehavior.h>

class EZ_PARTICLEPLUGIN_DLL ezParticleBehaviorFactory_Flies final : public ezParticleBehaviorFactory
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleBehaviorFactory_Flies, ezParticleBehaviorFactory);

public:
  ezParticleBehaviorFactory_Flies();
  ~ezParticleBehaviorFactory_Flies();

  virtual const ezRTTI* GetBehaviorType() const override;
  virtual void CopyBehaviorProperties(ezParticleBehavior* pObject, bool bFirstTime) const override;

  virtual void QueryFinalizerDependencies(ezSet<const ezRTTI*>& inout_finalizerDeps) const override;

  virtual void Save(ezStreamWriter& inout_stream) const override;
  virtual void Load(ezStreamReader& inout_stream) override;

  float m_fSpeed = 0.2f;
  float m_fPathLength = 0.2f;
  float m_fMaxEmitterDistance = 0.5f;
  ezAngle m_MaxSteeringAngle = ezAngle::MakeFromDegree(30);
};


class EZ_PARTICLEPLUGIN_DLL ezParticleBehavior_Flies final : public ezParticleBehavior
{
  EZ_ADD_DYNAMIC_REFLECTION(ezParticleBehavior_Flies, ezParticleBehavior);

public:
  virtual void CreateRequiredStreams() override;

  float m_fSpeed = 0.2f;
  float m_fPathLength = 0.2f;
  float m_fMaxEmitterDistance = 0.5f;
  ezAngle m_MaxSteeringAngle = ezAngle::MakeFromDegree(30);

protected:
  virtual void Process(ezUInt64 uiNumElements) override;

  ezProcessingStream* m_pStreamPosition = nullptr;
  ezProcessingStream* m_pStreamVelocity = nullptr;

  ezTime m_TimeToChangeDir;
};
