#pragma once

#include <QTreeWidget>

class SceneTreeWidget : public QTreeWidget
{
public:
    SceneTreeWidget();

    void mouseMoveEvent(QMouseEvent *evt) override;
};
