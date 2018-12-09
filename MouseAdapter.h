#pragma once

#include <memory>
#include <functional>
#include <vector>
#include <unordered_map>

#include <QMouseEvent>

#include "PipedEventAdapter.h"

#include "Core/geometry/Vector2.h"
#include "Core/util/WeakPointer.h"


class MouseAdapter {
public:

    enum class MouseEventType {
        ButtonPress = 0,
        ButtonRelease = 1,
        ButtonClick = 2,
        MouseMove = 3,
        WheelScroll = 4
    };

    class MouseEvent {
    public:
        MouseEvent(MouseEventType type): type(type) {}
        MouseEventType getType() {return  type;}
        Core::UInt32 buttons;
        Core::Vector2i position;
        Core::Real scrollDelta;
    private:
        MouseEventType type;
    };

    using ButtonEventCallback = std::function<void(MouseEventType, Core::UInt32, Core::UInt32, Core::UInt32)>;

    MouseAdapter();

    bool processEvent(QObject* obj, QEvent* event);
    bool setPipedEventAdapter(Core::WeakPointer<PipedEventAdapter<MouseEvent>> adapter);

    void onMouseButtonPressed(ButtonEventCallback callback);
    void onMouseButtonReleased(ButtonEventCallback callback);
    void onMouseButtonClicked(ButtonEventCallback callback);

private:
    class MouseButtonStatus {
    public:
        bool pressed;
        Core::Vector2i pressedLocation;
    };

    static const unsigned int MAX_BUTTONS = 16;
    MouseButtonStatus buttonStatuses[MAX_BUTTONS];
    unsigned int pressedButtonMask = 0;
    static unsigned int getMouseButtonIndex(const Qt::MouseButton& button);

    Core::WeakPointer<PipedEventAdapter<MouseEvent>> pipedEventAdapter;
    std::unordered_map<Core::UInt32, std::vector<ButtonEventCallback>> buttonEventCallbacks;
};

