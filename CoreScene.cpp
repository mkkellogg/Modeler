#include "CoreScene.h"

CoreScene::CoreScene() {

}

CoreScene::CoreScene(Core::WeakPointer<Core::Object3D> sceneRoot) {
    this->setSceneRoot(sceneRoot);
}

Core::WeakPointer<Core::Object3D> CoreScene::getSceneRoot() const {
    return this->sceneRoot;
}

void CoreScene::setSceneRoot(Core::WeakPointer<Core::Object3D> sceneRoot) {
    this->sceneRoot = sceneRoot;
}

void CoreScene::addObjectToScene(Core::WeakPointer<Core::Object3D> object) {
    this->addObjectToScene(object, this->sceneRoot);
}

void CoreScene::addObjectToScene(Core::WeakPointer<Core::Object3D> object, Core::WeakPointer<Core::Object3D> parent) {
    parent->addChild(object);
    for (auto& callback : this->sceneUpdatedCallbacks) {
        callback(object);
    }
}

void CoreScene::onSceneUpdated(SceneUpdatedCallback callback) {
    this->sceneUpdatedCallbacks.push_back(callback);
}
