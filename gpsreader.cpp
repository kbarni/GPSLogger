#include "gpsreader.h"

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <QThread>
#include <time.h>

/* ToDo:
 - More robust GPS port detection
 - Decode NMEA position after fix for monitoring
*/

GPSReader::GPSReader(QObject *parent) :
    QObject(parent)
{
    path = "";

    gpsport=new QSerialPort();

    QDir dev("/dev/serial/by-id/");
    QStringList filters;filters<<"usb-FTDI*";
    //dev.setNameFilters(filters);
    QStringList sl=dev.entryList(filters);
    if(sl.length()==0)return;
    QString port="/dev/serial/by-id/"+sl[0];
    //QString port="ttyUSB1";
    gpsport->setPortName(port);
    if(!gpsport->open(QIODevice::ReadWrite)) {
        qDebug()<<"Opening GPS on "+port+" -> PORT ERROR";
        havegps = false;
        return;
    }
    gpsport->setBaudRate(115200);
    gpsport->setDataBits(QSerialPort::Data8);
    gpsport->setParity(QSerialPort::NoParity);
    gpsport->setStopBits(QSerialPort::OneStop);
    gpsport->setFlowControl(QSerialPort::NoFlowControl);
    gpsport->clear();
    qDebug()<<"GPS opened on "+ port;
    havegps=true;
    numsats=0;
    chipset="Not available";
    messagecount = 0;
    goodtimestamp=false;
    valid=0;
    fileopened=false;
    incomplete=false;toread=0;
    connect(gpsport,SIGNAL(readyRead()),this,SLOT(ReadData()));
    return;
}

GPSReader::~GPSReader()
{
    if(gpsport->isOpen())gpsport->close();
    delete gpsport;
}

void GPSReader::stop()
{
    disconnect(gpsport);
    if(gpsport->isOpen())gpsport->close();
}

void GPSReader::ReadData()
{
    uint len;
    QByteArray ba;
    uptime = getUpTime();
    if(fileopened){ //File opened -> just write to file
        messagecount++;
        writebuffer.append((gpsport->readAll()));
        if(messagecount%100==0){
            glogfile.open(QFile::OpenModeFlag::Append);
            glogfile.write(writebuffer);
            glogfile.flush();
            qDebug()<<"Written stuff to file";
            glogfile.close();
            writebuffer.clear();
        }
    } else {
        while(gpsport->bytesAvailable()){
            if(incomplete){
                int curlen = ubxframe.size();
                ubxframe.append(gpsport->read(toread));
                toread -= (ubxframe.size()-curlen);
                if(toread>0) {
                    //qDebug()<<"Incomplete UBX message..."<<ubxframe.size()<<" "<<toread;
                    return;
                }
                incomplete=false;
                //qDebug()<<"Got UBX message\n  -->"<<binarymessage(ubxframe);
                decodeMessage(ubxframe);
            } else {
                ba = gpsport->read(6);
                if(ba.size()<6)return;
                if(((uchar)ba.at(0)==0xB5)&&((uchar)ba.at(1)==0x62)){ //UBX frame
                    len=(uchar)(ba.at(4))+((uchar)(ba.at(5))<<8)+2;
                    ubxframe = ba;
                    ubxframe.append(gpsport->read(len));
                    if(ubxframe.size()<len+6){
                        toread = len+6-ubxframe.size();
                        qDebug()<<"Incomplete UBX message..."<<ubxframe.size()<<" "<<toread;
                        incomplete = true;
                        return;
                    }
                    incomplete=false;
                    //qDebug()<<"Got UBX message\n  -->"<<binarymessage(ubxframe);
                    decodeMessage(ubxframe);
                }else if(ba.at(0)=='$'){
                    ba.append(gpsport->readLine());     //NMEA 0183 frame
                    //qDebug()<<"Got NMEA message\n  -->"<<QString::fromLocal8Bit(ba);
                } else{
                    ba.append(gpsport->readLine());     //Other frame
                    //qDebug()<<"*** Got other message\n  -->"<<binarymessage(ba);
                }
            }
        }
    }
}

void GPSReader::decodeMessage(QByteArray ba)
{
    char cls,id;
    cls=ba.at(2);id=ba.at(3);
    if((cls==0x01)&&(id==0x35)){    //Satellite number
        numsats = ba.at(11);
        qDebug()<<"  --> Got satellite number: "<<numsats;
    } else if((cls==0x01)&&(id==0x21)){    //UTC
        if(ba.size()<26){
            readTime();
            return;
        }
        valid = ba.at(25);
        qDebug()<<"  --> Got timestamp. Valid: "<<(uchar)valid;
        if((valid&4)==4){
            goodtimestamp = true;
            yr = (uchar)ba.at(18)+((uchar)ba.at(19)<<8);
            mo = (uchar)ba.at(20);
            dy = (uchar)ba.at(21);
            hr = (uchar)ba.at(22);
            mn = (uchar)ba.at(23);
            qDebug()<<"  --> Timestamp good! Current UTC time: "<<hr<<":"<<mn;
            filename = QString("trk_%1%2%3_%4%5.ubx").arg(yr,2,10,QChar('0')).arg(mo,2,10,QChar('0')).arg(dy,2,10,QChar('0')).arg(hr,2,10,QChar('0')).arg(mn,2,10,QChar('0'));
            glogfile.setFileName(path+filename);
            glogfile.open(QFile::OpenModeFlag::WriteOnly);
            qDebug()<<"Opening file "<<filename;
            if(!glogfile.isOpen())
                qDebug()<<"****File not opened!****";
            glogfile.close();
            fileopened=true;
            messagecount=0;
            writebuffer.clear();
        } else {
            readTime();
            QThread::msleep(200);
            readSats();
        }
    } else if((cls==0x0A)&&(id==0x04)){    //Chipset
        if(ba.size()<45){   //message too short
            readVersion();
            return;
        }
        chipset = QString(ba.mid(36,8));
        qDebug()<<"  --> Got chipset version: "<<chipset;
    }
}

void GPSReader::readVersion()
{
    if(!havegps)return;
    QByteArray payload = QByteArrayLiteral("\x0a\x04\x00\x00");
    QByteArray msg = QByteArrayLiteral("\xb5\x62")+payload+calc_crc(payload);
    gpsport->write(msg);
}

void GPSReader::readTime()
{
    if(!havegps)return;
    QByteArray payload = QByteArrayLiteral("\x01\x21\x00\x00");
    QByteArray msg = QByteArrayLiteral("\xb5\x62")+payload+calc_crc(payload);
    gpsport->write(msg);
}

void GPSReader::readSats()
{
    if(!havegps)return;
    QByteArray payload = QByteArrayLiteral("\x01\x35\x00\x00");
    QByteArray msg = QByteArrayLiteral("\xb5\x62")+payload+calc_crc(payload);
    gpsport->write(msg);
}

QByteArray GPSReader::calc_crc(QByteArray in)
{
    long cka,ckb;
    cka=0;ckb=0;
    for(auto b:in){
        cka += b;
        ckb += cka;
    }
    char cha[2];
    cha[0]=(char)(cka&0xFF);
    cha[1]=(char)(ckb&0xFF);
    return QByteArray(cha,2);
}

QString GPSReader::lastMessage()
{
    QString message="GpsData: ["+QString::number(getUpTime())+"]";
    if(goodtimestamp)message+="\nValid UTC time found: "+QString::number(hr)+":"+QString::number(mn);
    else message+="Visible satellites :"+QString::number(numsats)+"\nUTC time validity: "+QString::number(valid);
    if(fileopened)message+="\nWriting to file: "+filename+"\nMessages written: "+QString::number(messagecount);
    message+="\nLast message timestamp: "+QString::number(uptime);
    return message;
}

QString GPSReader::chipVersion()
{
    return chipset;
}

bool GPSReader::hasGPS()
{
    return havegps;
}

QString GPSReader::binarymessage(QByteArray ba)
{
    QString s;
    for(auto c : ba){
        s+=QString(" %1").arg((uchar)c,2,16,QChar('0'));
    }
    return s;
}

long GPSReader::getUpTime()
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC,&t);
    return t.tv_sec;
}
