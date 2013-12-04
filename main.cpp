#include <QCoreApplication>
#include <QRegExp>
#include <QStringList>
#include <QDebug>

#include "inputdevice.h"
#include "keystreamserver.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    int port = 7856;

    QRegExp exPort("--port=(\\d+)");

    foreach (auto arg, app.arguments()) {
        if (arg.contains(exPort)) {
            port = exPort.cap(1).toInt();
        } else if (arg == "--help") {
            printf("Usage: %s --port=<port number>\n", qPrintable(app.applicationFilePath()));
            return 0;
        }
    }

    InputDevice dev;
    if (!dev.create("event-input-daemon")) {
        qCritical() << "Failed to create input device";
        return -1;
    }

    KeyStreamServer server;
    if (!server.listen(port)) {
        qCritical() << "Failed to start key stream server";
        return -1;
    }

    bool ok = QObject::connect(&server, &KeyStreamServer::keyEventReceived,
                               &dev, &InputDevice::sendKeyEvent);
    Q_ASSERT(ok);
    Q_UNUSED(ok);

    printf("Listening at port %d\n", port);

    return app.exec();
}
