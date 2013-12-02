#ifndef INPUTDEVICE_H
#define INPUTDEVICE_H

#include <QObject>

class InputDevicePrivate;

class InputDevice : public QObject {
    Q_OBJECT
public:
    InputDevice(QObject *parent = 0);
    ~InputDevice();

    bool create(const QString &name);
    void destroy();

public slots:
    bool sendKeyEvent(Qt::Key key, bool pressed);

private:
    Q_DECLARE_PRIVATE(InputDevice)
    InputDevicePrivate* const d_ptr;
};

#endif // INPUTDEVICE_H
