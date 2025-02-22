#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Utilities/EnumerableClass.h>

/// \brief Describes the different stages during startup and shutdown
struct ezStartupStage
{
  enum Enum
  {
    None = -1,
    BaseSystems,      ///< In this stage the absolute base functionality is started. This should only be used by the Foundation library.
    CoreSystems,      ///< In this stage the core functionality is being started / shut down
    HighLevelSystems, ///< In this stage the higher level functionality, which depends on a rendering context, is being started / shut down

    ENUM_COUNT
  };
};

/// \brief Base class for all subsystems.
///
/// ezStartup will initialize and shut down all instances of this class, according to their dependencies etc.
/// If you have a subsystem that is a non-static class, just derive from this base class and override the
/// virtual functions as required.
/// If you have a subsystem that is implemented in a purely static way (there is no class instance),
/// just use the macros EZ_BEGIN_SUBSYSTEM_DECLARATION, EZ_END_SUBSYSTEM_DECLARATION etc.
/// Those macros will create a wrapper object (derived from ezSubSystem) to handle initialization.
class EZ_FOUNDATION_DLL ezSubSystem : public ezEnumerable<ezSubSystem>
{
  EZ_DECLARE_ENUMERABLE_CLASS(ezSubSystem);
  EZ_DISALLOW_COPY_AND_ASSIGN(ezSubSystem);

public:
  ezSubSystem()
  {
    for (ezInt32 i = 0; i < ezStartupStage::ENUM_COUNT; ++i)
      m_bStartupDone[i] = false;
  }

  virtual ~ezSubSystem() = default;

  /// \brief Returns the name of the subsystem. Must be overridden.
  virtual ezStringView GetSubSystemName() const = 0;

  /// \brief Returns the name of the group to which this subsystem belongs. Must be overridden.
  virtual ezStringView GetGroupName() const = 0;

  /// \brief Returns a series of strings with the names of the subsystem, which this subsystem depends on. nullptr indicates the last entry.
  /// Must be overridden.
  virtual ezStringView GetDependency(ezInt32 iDep)
  {
    EZ_IGNORE_UNUSED(iDep);
    return {};
  }

  /// \brief Returns the plugin name to which this subsystem belongs.
  ezStringView GetPluginName() const { return m_sPluginName; }

  /// \brief Returns whether the given startup stage has been done on this subsystem.
  bool IsStartupPhaseDone(ezStartupStage::Enum stage) const { return m_bStartupDone[stage]; }

private:
  // only the startup system may access the following functionality
  friend class ezStartup;

  /// \brief This will be called to initialize the subsystems base components. Can be overridden to handle this event.
  virtual void OnBaseSystemsStartup() {}

  /// \brief This will be called to initialize the subsystems core components. Can be overridden to handle this event.
  virtual void OnCoreSystemsStartup() {}

  /// \brief This will be called to shut down the subsystems core components. Can be overridden to handle this event.
  virtual void OnCoreSystemsShutdown() {}

  /// \brief This will be called to initialize the subsystems engine / rendering components. Can be overridden to handle this event.
  virtual void OnHighLevelSystemsStartup() {}

  /// \brief This will be called to shut down the subsystems engine / rendering components. Can be overridden to handle this event.
  virtual void OnHighLevelSystemsShutdown() {}

  /// Set by ezStartup to store to which plugin this subsystem belongs.
  ezStringView m_sPluginName;

  /// Stores which startup phase has been done already.
  bool m_bStartupDone[ezStartupStage::ENUM_COUNT];
};

#include <Foundation/Configuration/StaticSubSystem.h>
