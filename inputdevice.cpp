#include "inputdevice.h"

#include <QMap>
#include <QDebug>

#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>

class InputDevicePrivate : public QObject {
public:
    InputDevicePrivate() : created(false) {
        resetFd();
        idKeyMap[Qt::Key_PowerOff] = KEY_POWER;
        idKeyMap[Qt::Key_VolumeDown] = KEY_VOLUMEDOWN;
        idKeyMap[Qt::Key_VolumeUp] = KEY_VOLUMEUP;
    }

    void resetFd() {
        fd = -1;
    }

    bool isFdValid() const {
        return fd >= 0;
    }

    int qtKeyToUiKey(Qt::Key key) const {
        Q_ASSERT(idKeyMap.contains(key));
        return idKeyMap.value(key, -1);
    }

    bool sendUinputEvent(uint type, uint code, int value) {
        input_event ev;
        memset(&ev, 0, sizeof(ev));
        ev.type = type;
        ev.code = code;
        ev.value = value;

        if (write(fd, &ev, sizeof(ev)) != sizeof(ev)) {
            qWarning() << "Failed to send uinput event";
            return false;
        }
        return true;
    }

    bool created;
    int fd;
    QMap<Qt::Key, int> idKeyMap;
};

InputDevice::InputDevice(QObject *parent)
    : QObject(parent), d_ptr(new InputDevicePrivate) {
}

InputDevice::~InputDevice() {
    destroy();
    delete d_ptr;
}

bool InputDevice::create(const QString &name) {
    Q_D(InputDevice);

    d->fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (!d->isFdValid()) {
        qWarning() << "Failed to open '/dev/uinput'";
        return false;
    }

    foreach (Qt::Key key, d->idKeyMap.keys().toSet()) {
        if (ioctl(d->fd, UI_SET_EVBIT, EV_KEY) < 0) {
            qWarning() << "Failed to enable keys for the uinput device";
            return false;
        }

        if (ioctl(d->fd, UI_SET_KEYBIT, d->qtKeyToUiKey(key)) < 0) {
            qWarning() << "Failed to enable Qt key" << key;
            return false;
        }
    }

    uinput_user_dev uidev;
    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, qPrintable(name));
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x0012;
    uidev.id.product = 0xcafe;
    uidev.id.version = 1;
    if (write(d->fd, &uidev, sizeof(uidev)) != sizeof(uidev)) {
        qWarning() << "Failed to write device information to uinput";
        return false;
    }

    if (ioctl(d->fd, UI_DEV_CREATE) < 0) {
        qWarning() << "Failed to create uinput device";
        return false;
    }

    d->created = true;
    return true;
}

void InputDevice::destroy() {
    Q_D(InputDevice);
    if (d->isFdValid()) {
        ioctl(d->fd, UI_DEV_DESTROY);
        close(d->fd);
    }
    d->resetFd();
    d->created = false;
}

bool InputDevice::sendKeyEvent(Qt::Key key, bool pressed) {
    Q_D(InputDevice);

    if (!d->created) {
        qWarning() << "Cannot send key event, device has not been created";
        return false;
    }

    if (!d->sendUinputEvent(EV_KEY, d->qtKeyToUiKey(key), pressed? 1: 0)) {
        qWarning() << "Failed to send key" << key;
        return false;
    }

    if (!d->sendUinputEvent(EV_SYN, 0, 0)) {
        qWarning() << "Failed to send sync event";
        return false;
    }

    qDebug() << "key event sent to uinput device succesfully";

    return true;
}
