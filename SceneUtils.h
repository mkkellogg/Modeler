#pragma once

#include <vector>

#include "Core/Engine.h"
#include "Core/scene/Object3D.h"

class SceneUtils
{
public:
    SceneUtils();
    static void getRootObjects(const std::vector<Core::WeakPointer<Core::Object3D>>& objects, std::vector<Core::WeakPointer<Core::Object3D>>& roots);
};
