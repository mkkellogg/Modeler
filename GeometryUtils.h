#pragma once

#include "Core/Engine.h"
#include "Core/common/types.h"
#include "Core/color/Color.h"

class Mesh;

class GeometryUtils
{
public:
    GeometryUtils();
    Core::WeakPointer<Mesh> buildArrowMesh(Core::Real baseLength, Core::Real baseRadius,
                                               Core::Real coneLength, Core::Real coneRadius,
                                               Core::UInt32 subdivisions, Core::Color color);
};


