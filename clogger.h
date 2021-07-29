#ifndef CLOGGER_H
#define CLOGGER_H

#include <QObject>
#include "myserver.h"
#include "gpsreader.h"

class CLogger : public QObject
{
    Q_OBJECT
public:
    explicit CLogger(QObject *parent = nullptr);
    ~CLogger();

private:
    MyServer *server;
    GPSReader *reader;
    QTimer *updatetimer;

    QString version;

signals:
    void quit();

public slots:
    void newConnection();
    void clientLeft();
    void gotMessage(QString message);
    void updateStatus();

};

#endif // CLOGGER_H
