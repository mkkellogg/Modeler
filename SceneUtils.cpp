#include "SceneUtils.h"

SceneUtils::SceneUtils() {

}

void SceneUtils::getRootObjects(const std::vector<Core::WeakPointer<Core::Object3D>>& objects, std::vector<Core::WeakPointer<Core::Object3D>>& roots) {
    for (unsigned int i = 0; i < objects.size(); i++)  {
        Core::WeakPointer<Core::Object3D> object = objects[i];
        Core::WeakPointer<Core::Object3D> parent = object->getParent();
        bool isRoot = true;
        while (isRoot && parent) {
            for (unsigned int j = 0; j < objects.size(); j++)  {
                if(objects[j]->getID() == parent->getID()) {
                    isRoot = false;
                    break;
                }
            }
            parent = parent->getParent();
        }
        if (isRoot) {
            roots.push_back(object);
        }
    }
}
