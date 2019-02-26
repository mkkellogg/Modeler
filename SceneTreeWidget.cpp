#include "SceneTreeWidget.h"

SceneTreeWidget::SceneTreeWidget() {

}

void SceneTreeWidget::mouseMoveEvent(QMouseEvent *evt) {

}

QTreeWidgetItem * SceneTreeWidget::getItemFromIndex(const QModelIndex & index) {
    return this->itemFromIndex(index);
}
