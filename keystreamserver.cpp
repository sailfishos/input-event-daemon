#include "keystreamserver.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QDataStream>
#include <QStringList>
#include <functional>

namespace {

Qt::Key keyNameToKey(const QString &name) {
    static QMap<QString, Qt::Key> map;
    if (map.isEmpty()) {
        map["power-off"] = Qt::Key_PowerOff;
        map["volume-down"] = Qt::Key_VolumeDown;
        map["volume-up"] = Qt::Key_VolumeUp;
    }
    Q_ASSERT(map.contains(name));
    return map[name];
}

}

class MessageReader {
public:
    MessageReader(QTcpSocket *connection, std::function<void(QByteArray)> callback)
        : connection(connection), callback(callback) {
        QObject::connect(connection, &QTcpSocket::readyRead, [this]() {
            handleReadyRead();
        });

        handler = std::bind(&MessageReader::handleSize, this);
    }

    void handleReadyRead() {
        while (connection->bytesAvailable()) {
            handler();
        }
    }

    void handleSize() {
        if (connection->bytesAvailable() >= sizeof(messageSize)) {
            auto data = connection->read(2);
            QDataStream stream(&data, QIODevice::ReadOnly);
            stream >> messageSize;
            qDebug() << "MESSAGE SIZE:" << messageSize;
            handler = std::bind(&MessageReader::handleMessage, this);
        }
    }

    void handleMessage() {
        if (connection->bytesAvailable()) {
            message += connection->read(messageSize - message.size());

            if (message.size() == messageSize) {
                callback(message);
                message.clear();
                handler = std::bind(&MessageReader::handleSize, this);
            }
        }
    }

    QTcpSocket *connection;
    std::function<void(QByteArray)> callback;
    std::function<void()> handler;
    quint16 messageSize;
    QByteArray message;
};

class KeyStreamServerPrivate {
public:
    KeyStreamServerPrivate(KeyStreamServer *q) : q_ptr(q) {

    }

    void onNewConnection() {
        Q_Q(KeyStreamServer);

        auto connection = tcpServer->nextPendingConnection();
        Q_ASSERT(connection);
        qDebug() << "New connection received";
        // TODO: handle deletion of MessageReader
        new MessageReader(connection, [q](const QByteArray &message) {
            qDebug() << "Received message:" << message.toHex();
            auto msgStr = QString::fromUtf8(message);
            auto parts = msgStr.split(':');
            Q_ASSERT(parts.length());

            if (parts[0] == "key-event" && parts.length() == 3) {
                emit q->keyEventReceived(keyNameToKey(parts[1]), parts[2].toInt());
            }
        });
    }

    QTcpServer *tcpServer;

    Q_DECLARE_PUBLIC(KeyStreamServer)
    KeyStreamServer* const q_ptr;
};

typedef KeyStreamServerPrivate Pimpl;

KeyStreamServer::KeyStreamServer(QObject *parent)
    : QObject(parent), d_ptr(new KeyStreamServerPrivate(this)) {
    Q_D(KeyStreamServer);
    d->tcpServer = new QTcpServer(this);

    connect(d->tcpServer, &QTcpServer::newConnection, [d]() {
        d->onNewConnection();
    });
}

KeyStreamServer::~KeyStreamServer() {
    stop();
    delete d_ptr;
}

bool KeyStreamServer::listen(int port) {
    Q_D(KeyStreamServer);
    if (!d->tcpServer->listen(QHostAddress::Any, port)) {
        qWarning() << "Failed to start tcp server at port" << port;
        return false;
    }
    return true;
}

void KeyStreamServer::stop() {
    Q_D(KeyStreamServer);
    d->tcpServer->close();
}
