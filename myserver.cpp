#include "myserver.h"

#include <QDebug>

//#define FUTURES
#ifdef FUTURES
#include <QtConcurrent/QtConcurrent>
#endif

MyServer::MyServer(QString servername, int port, QObject *parent) : QObject(parent)
{
    wsServer = new QWebSocketServer(servername,QWebSocketServer::NonSecureMode);
    opened=false;
    if (!wsServer->listen(QHostAddress::Any, port)) {
        qDebug()<<"MyServer: Unable to start the server:"<<wsServer->errorString();
        return;
    }
    port=wsServer->serverPort();
    connect(wsServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
    opened=true;
    imagecounter=0;
    sending=false;
}

MyServer::~MyServer()
{
    if(opened)
        Close();
    delete wsServer;
}

void MyServer::Close()
{
    qDebug()<<"Closing server";
    wsServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
    opened = false;
}
void MyServer::newConnection()
{
    QWebSocket *pSocket = wsServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &MyServer::readyRead);
    connect(pSocket, &QWebSocket::disconnected, this, &MyServer::clientLeft);

    m_clients << pSocket;
    emit clientConnected();
}


bool MyServer::isServerListening(){return opened;}

void MyServer::clientLeft()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug() << "socketDisconnected:" << pClient;
    if (pClient) {
        pClient->disconnect();
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}

void MyServer::readyRead(QString message)
{
    qDebug()<<"Message received: "<<message;
    emit gotMessage(message);
}

bool MyServer::sendMessage(QString msg)
{
    if(m_clients.size()==0)return false;
    for(auto client : m_clients){
#ifdef FUTURES
        QFuture<qint64> future = QtConcurrent::run(client,&QWebSocket::sendTextMessage,msg);
        future.waitForFinished();
#else
        client->sendTextMessage(msg);
#endif
    }
    qDebug()<<"Sending: "+msg;
    return true;
}

bool MyServer::sendMessage(QString cmd, QString param)
{
    return sendMessage(cmd+" "+param);
}

void MyServer::sendImage(QByteArray ba)
{
    for(auto client : m_clients)
        client->sendBinaryMessage(ba);
}
