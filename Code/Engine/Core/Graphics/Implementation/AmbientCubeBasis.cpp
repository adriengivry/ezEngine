#include <Core/CorePCH.h>

#include <Core/Graphics/AmbientCubeBasis.h>

ezVec3 ezAmbientCubeBasis::s_Dirs[NumDirs] = {ezVec3(1.0f, 0.0f, 0.0f), ezVec3(-1.0f, 0.0f, 0.0f), ezVec3(0.0f, 1.0f, 0.0f),
  ezVec3(0.0f, -1.0f, 0.0f), ezVec3(0.0f, 0.0f, 1.0f), ezVec3(0.0f, 0.0f, -1.0f)};
