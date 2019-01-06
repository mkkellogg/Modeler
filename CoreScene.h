#pragma once

#include <vector>
#include <functional>

#include "Core/Engine.h"

class CoreScene {
public:
    using SceneUpdatedCallback = std::function<void(Core::WeakPointer<Core::Object3D>)>;

    CoreScene();
    CoreScene(Core::WeakPointer<Core::Object3D> sceneRoot);
    Core::WeakPointer<Core::Object3D> getSceneRoot() const;
    void setSceneRoot(Core::WeakPointer<Core::Object3D> sceneRoot);
    void addObjectToScene(Core::WeakPointer<Core::Object3D> object);
    void addObjectToScene(Core::WeakPointer<Core::Object3D> object, Core::WeakPointer<Core::Object3D> parent);
    void onSceneUpdated(SceneUpdatedCallback callback);

private:
    Core::WeakPointer<Core::Object3D> sceneRoot;
    std::vector<SceneUpdatedCallback> sceneUpdatedCallbacks;
};
