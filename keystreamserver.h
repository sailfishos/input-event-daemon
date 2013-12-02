#ifndef KEYSTREAMSERVER_H
#define KEYSTREAMSERVER_H

#include <QObject>

class KeyStreamServerPrivate;

class KeyStreamServer : public QObject {
    Q_OBJECT
public:
    KeyStreamServer(QObject *parent = 0);
    ~KeyStreamServer();

    bool listen(int port);
    void stop();

signals:
    void keyEventReceived(Qt::Key key, bool pressed);

private:
    Q_DECLARE_PRIVATE(KeyStreamServer)
    KeyStreamServerPrivate* const d_ptr;
};

#endif // KEYSTREAMSERVER_H
