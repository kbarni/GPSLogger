#include "clogger.h"

/// ToDo:
/// - Send chipset separately
/// -

CLogger::CLogger(QObject *parent) : QObject(parent)
{
    version = "0.9-RC";
    qDebug()<<"GPSLogger version "<<version;

    reader = new GPSReader();
    server = new MyServer("GPSLogger",1215);

    connect(server,SIGNAL(clientConnected()),this,SLOT(newConnection()));
    connect(server,SIGNAL(clientDisconnected()),this,SLOT(clientLeft()));
    connect(server,SIGNAL(gotMessage(QString)),this,SLOT(gotMessage(QString)));

    updatetimer = new QTimer(this);
    connect(updatetimer,SIGNAL(timeout()),this,SLOT(updateStatus()));
    updatetimer->start(2000);

    //reader->openPort();
    QThread::msleep(500);
    reader->readVersion();
    QThread::msleep(200);
    reader->readTime();
}

CLogger::~CLogger()
{
    //server->close();
    emit quit();
}

void CLogger::newConnection()
{

    server->sendMessage("chipset",reader->chipVersion());
    server->sendMessage("version",version);
}

void CLogger::clientLeft()
{

}

void CLogger::gotMessage(QString message)
{
    if(message=="reopen"){

    }
    if(message=="reset"){
        reader->readTime();
    }
    if(message=="stop"){
        reader->stop();
    }
}

void CLogger::updateStatus()
{
    server->sendMessage(reader->lastMessage());
}
