#pragma once

#include "Core/Engine.h"
#include "Core/common/types.h"
#include "Core/color/Color.h"
#include "Core/render/BaseRenderable.h"

class Mesh;

class GeometryUtils
{
public:
    GeometryUtils();

    static Core::WeakPointer<Core::Mesh> buildBoxMesh(Core::Real length, Core::Real width, Core::Real depth, Core::Color color);

    static Core::WeakPointer<Core::Mesh> buildArrowMesh(Core::Real baseLength, Core::Real baseRadius,
                                                        Core::Real coneLength, Core::Real coneRadius,
                                                        Core::UInt32 subdivisions, Core::Color color);
    static Core::WeakPointer<Core::RenderableContainer<Core::Mesh>> buildMeshContainer(Core::WeakPointer<Core::Mesh> mesh,
                                                                Core::WeakPointer<Core::Material> material,
                                                                const std::string& name);
};


