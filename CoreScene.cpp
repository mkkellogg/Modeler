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

std::vector<Core::WeakPointer<Core::Object3D>>& CoreScene::getSelectedObjects() {
    return this->selectedObjects;
}

void CoreScene::addSelectedObject(Core::WeakPointer<Core::Object3D> newSelectedObject) {
    if (newSelectedObject.isValid() && !this->isObjectSelected(newSelectedObject)) {
        this->selectedObjects.push_back(newSelectedObject);
        for (OnObjectSelectedCallback callback : this->selectedObjectAddedCallbacks) {
            callback(newSelectedObject);
        }
    }
}

void CoreScene::removeSelectedObject(Core::WeakPointer<Core::Object3D> objectToRemove) {
    if (this->selectedObjects.size() > 0) {
        for (unsigned int i = 0; i < this->selectedObjects.size(); i++) {
            Core::WeakPointer<Core::Object3D> selectedObject = this->selectedObjects[i];
            if(selectedObject.get() == objectToRemove.get()) {
                this->selectedObjects[i] = this->selectedObjects[this->selectedObjects.size() - 1];
                this->selectedObjects.pop_back();
                for (OnObjectSelectedCallback callback : this->selectedObjectRemovedCallbacks) {
                    callback(objectToRemove);
                }
                return;
            }
        }
    }
}

void CoreScene::onSelectedObjectAdded(OnObjectSelectedCallback callback) {
    this->selectedObjectAddedCallbacks.push_back(callback);
}

void CoreScene::onSelectedObjectRemoved(OnObjectSelectedCallback callback) {
    this->selectedObjectRemovedCallbacks.push_back(callback);
}

bool CoreScene::isObjectSelected(Core::WeakPointer<Core::Object3D> candidateObject) {
    for (std::vector<Core::WeakPointer<Core::Object3D>>::iterator itr = this->selectedObjects.begin(); itr != this->selectedObjects.end(); ++itr) {
        Core::WeakPointer<Core::Object3D> selectedObject = *itr;
        if(selectedObject.get() == candidateObject.get()) return true;
    }
    return false;
}
