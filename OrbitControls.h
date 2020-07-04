#pragma once

#include "GestureAdapter.h"
#include "Core/Engine.h"
#include "Core/geometry/Vector3.h"
#include "CoreSync.h"


class OrbitControls {
public:
    OrbitControls(Core::WeakPointer<Core::Engine> engine, Core::WeakPointer<Core::Camera> targetCamera, Core::WeakPointer<CoreSync> coreSync);
    void handleGesture(GestureAdapter::GestureEvent event);
    void setOrigin(Core::Real x, Core::Real y, Core::Real z);
    Core::Point3r getOrigin();
    void resetMove();
private:
    Core::UInt32 moveFrames;
    Core::Point3r origin;
    Core::WeakPointer<Core::Engine> engine;
    Core::WeakPointer<Core::Camera> targetCamera;
    Core::WeakPointer<CoreSync> coreSync;
    Core::Int32 lastMoveX;
    Core::Int32 lastMoveY;
};

