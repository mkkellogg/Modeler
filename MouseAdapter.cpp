#include "Core/util/WeakPointer.h"

#include "MouseAdapter.h"
#include "Settings.h"

#include <QApplication>
#include <QDesktopWidget>

MouseAdapter::MouseAdapter() {

}

bool MouseAdapter::setPipedEventAdapter(Core::WeakPointer<PipedEventAdapter<MouseEvent>> adapter) {
    if(!this->pipedEventAdapter) {
        this->pipedEventAdapter = adapter;
        return true;
    }
    else {
        return false;
    }
}

void MouseAdapter::onMouseMoved(MoveEventCallback callback) {
    this->moveEventCallbacks.push_back(callback);
}

void MouseAdapter::onMouseButtonPressed(ButtonEventCallback callback) {
    this->buttonEventCallbacks[(Core::UInt32)MouseEventType::ButtonPress].push_back(callback);
}

void MouseAdapter::onMouseButtonReleased(ButtonEventCallback callback) {
    this->buttonEventCallbacks[(Core::UInt32)MouseEventType::ButtonRelease].push_back(callback);
}

void MouseAdapter::onMouseButtonClicked(ButtonEventCallback callback) {
    this->buttonEventCallbacks[(Core::UInt32)MouseEventType::ButtonClick].push_back(callback);
}

bool MouseAdapter::processEvent(QObject* obj, QEvent* event) {
    float dpr = QApplication::desktop()->devicePixelRatio();
    auto eventType = event->type();
    if (eventType == QEvent::MouseButtonPress ||
        eventType == QEvent::MouseButtonRelease ||
        eventType == QEvent::MouseMove) {

        const QMouseEvent* const mouseEvent = static_cast<const QMouseEvent*>( event );

        QPoint qMousePos = mouseEvent->pos();
        Core::Int32 scaledX = static_cast<int>(static_cast<float>(qMousePos.x()) * dpr);
        Core::Int32 scaledY = static_cast<int>(static_cast<float>(qMousePos.y()) * dpr);
        Core::Vector2i mousePos(scaledX, scaledY);

        MouseEventType mouseEventType;
        switch(eventType) {
            case QEvent::MouseButtonPress:
            {
                unsigned int buttonIndex = getMouseButtonIndex(mouseEvent->button());
                buttonStatuses[buttonIndex].pressed = true;
                buttonStatuses[buttonIndex].pressedLocation = mousePos;
                pressedButtonMask |= 1 << (buttonIndex - 1);
                mouseEventType = MouseEventType::ButtonPress;
                std::vector<ButtonEventCallback> pressCallbacks = this->buttonEventCallbacks[(Core::UInt32)MouseEventType::ButtonPress];
                for (ButtonEventCallback callback : pressCallbacks) {
                    callback(MouseEventType::ButtonPress, buttonIndex, mousePos.x, mousePos.y);
                }
                break;
            }
            case QEvent::MouseButtonRelease:
            {
                unsigned int buttonIndex = getMouseButtonIndex(mouseEvent->button());
                buttonStatuses[buttonIndex].pressed = false;
                pressedButtonMask &= ~(1 << (buttonIndex - 1));
                mouseEventType = MouseEventType::ButtonRelease;
                std::vector<ButtonEventCallback> pressCallbacks = this->buttonEventCallbacks[(Core::UInt32)MouseEventType::ButtonRelease];
                for (ButtonEventCallback callback : pressCallbacks) {
                    callback(MouseEventType::ButtonRelease, buttonIndex, mousePos.x, mousePos.y);
                }
                break;
            }
            case QEvent::MouseMove:
                mouseEventType = MouseEventType::MouseMove;
                for (MoveEventCallback callback : this->moveEventCallbacks) {
                    callback(mousePos.x, mousePos.y);
                }
                break;
            default: break;
        }
        if (this->pipedEventAdapter) {
            MouseEvent event(mouseEventType);
            event.buttons = pressedButtonMask;
            event.position = mousePos;
            Core::WeakPointer<PipedEventAdapter<MouseEvent>> adapterPtr(this->pipedEventAdapter);
            adapterPtr->accept(event);
        }
        return true;
    }
    else if (eventType == QEvent::Wheel ) {

         const QWheelEvent* const wheelEvent = static_cast<const QWheelEvent*>( event );
         if (this->pipedEventAdapter) {
             MouseEvent event(MouseEventType::WheelScroll);
             QPoint pDelta = wheelEvent->angleDelta();

             event.scrollDelta = (Core::Real)wheelEvent->delta() / 240.0f;
             event.buttons = 0;
             Core::WeakPointer<PipedEventAdapter<MouseEvent>> adapterPtr(this->pipedEventAdapter);
             adapterPtr->accept(event);
         }
    }

     return false;
}

unsigned int MouseAdapter::getMouseButtonIndex(const Qt::MouseButton& button) {
    if(button == Qt::LeftButton) {return 1;}
    else if(button == Qt::RightButton) {return 2;}
    else if((button == Qt::MiddleButton) || (button == Qt::MidButton || button == Settings::AltMiddleButton)) {return 3;}
    else if(button == Qt::XButton1){return 4;}
    else if(button == Qt::XButton2){return 5;}
    else return 0;
}

