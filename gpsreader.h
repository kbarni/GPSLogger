#ifndef SENSORS_H
#define SENSORS_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QByteArray>
#include <QFile>

class GPSReader : public QObject
{
    Q_OBJECT
public:
    explicit GPSReader(QObject *parent = 0);
    ~GPSReader();

    void stop();

    bool openPort();
    void readVersion();
    void readTime();
    void readSats();

    bool hasGPS();
    QString chipVersion();
    QString lastMessage();

private:
    QString filename;
    QString lastmessage;
    bool fileopened,goodtimestamp;
    bool havegps;
    QString chipset;
    QString path;
    int numsats,valid,messagecount;
    int uptime,yr,mo,dy,hr,mn;
    QSerialPort *gpsport;

    QByteArray calc_crc(QByteArray in);

    void decodeMessage(QByteArray ba);

    QString binarymessage(QByteArray ba);

    long getUpTime();

    QByteArray ubxframe,writebuffer;
    QFile glogfile;
    bool incomplete;
    int toread;

signals:

public slots:
    void ReadData();
};

#endif // SENSORS_H
