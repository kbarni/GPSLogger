#ifndef MYSERVER_H
#define MYSERVER_H

#include <QObject>
#include <QtWebSockets/QtWebSockets>

class MyServer : public QObject
{
    Q_OBJECT
public:
    MyServer(QString servername, int port, QObject *parent = 0);
    ~MyServer();
    bool sendMessage(QString msg);
    bool sendMessage(QString cmd, QString param);

    bool isServerListening();
    void Close();
private:
    bool opened;
    QWebSocketServer *wsServer;
    QList<QWebSocket *> m_clients;
    bool sending;
    int imagecounter;

signals:
    void clientConnected();
    void clientDisconnected();
    void gotMessage(QString message);
public slots:
    void newConnection();
    void readyRead(QString message);
    void clientLeft();

    void sendImage(QByteArray ba);
};

#endif // MYSERVER_H
