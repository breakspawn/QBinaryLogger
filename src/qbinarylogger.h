#ifndef QBINARYLOGGER_H
#define QBINARYLOGGER_H

#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QMap>
#include <QByteArray>
#include <QTextStream>
#include <QtEndian>
#include "QDebug"
#include "QDataStream"
#include <quazip.h>
#include <JlCompress.h>


class QBinaryLogger {
public:
#pragma pack(1)
    struct header {
        uint64_t m_tm;
        uint16_t m_len;

        header()
            : m_tm(0), m_len(0)
        {

        }

        header(uint16_t datasize)
        {
            uint64_t t = QDateTime::currentDateTime().toTime_t() | ((uint64_t)(QTime::currentTime().msecsSinceStartOfDay() % 1000) << 54);
            m_tm = qToLittleEndian<uint64_t>(t);

            m_len = qToLittleEndian<uint16_t>(datasize);
        }

        uint64_t time() const {

            uint64_t t = qFromLittleEndian<uint64_t>(m_tm);
            return  t & ~((uint64_t)(0xFFC0) << 48);
        }

        uint64_t ms() const {
            uint64_t ms =  qFromLittleEndian<uint64_t>(m_tm);
            return ms >> 54;
        }

        uint16_t len() const {
            return qFromLittleEndian<uint16_t>(m_len);
        }
    };
#pragma pack()

    enum log_mode_t {
        ZIP,
        TRUNCATE
    };

    struct Note {
        Note()
        {

        }

        Note(const QByteArray & data)
            : h(data.size()), d(data)
        {

        }

        QByteArray toByteArray() const
        {
            return QByteArray((const char*)&h, sizeof h) + d;
        }

        Note& operator<<(QDataStream & ds)
        {
            header hdr;
            if(ds.readRawData((char*)&hdr, sizeof hdr) == sizeof hdr) {
                d = QByteArray(hdr.len(), 0);

                if(ds.readRawData(d.data(), d.size()) == d.size())
                {
                    h = hdr;
                }
            }

            return *this;
        }

        header h;
        QByteArray d;
    };

    static QList<Note> read(const QString & filepath) {
        QList<Note> notes;

        QFile f(filepath);
        if(f.exists() && f.open(QIODevice::ReadOnly))
        {
            QDataStream ts(&f);

            while(!ts.atEnd())
            {
                Note n;
                n << ts;
                if(n.h.time() && n.h.len() == n.d.length())
                    notes.append(n);
            }
        }

        return notes;
    }

    QString sFilePath;
    QDir    dir;
    QString sFileNameWithoutExt;
    QString sExt;

    QDateTime lastWrite;
    log_mode_t mode;

    QBinaryLogger(const QString &filename, log_mode_t mode = TRUNCATE)
    {
        this->mode = mode;

        sFilePath = filename;

        QFileInfo fi(filename);
        if(fi.isRelative())
        {
            if(!fi.dir().exists())
            {
                fi.dir().mkpath(fi.absolutePath());
            }
            sFilePath = fi.absoluteFilePath();
        }
        sFileNameWithoutExt = fi.baseName();
        sExt = fi.completeSuffix();
        dir  = fi.dir();
        if(mode == TRUNCATE)
        removeDublicates(fileName());
        if(mode == ZIP)
            toArhive(fileName());
    }

    QString fileName() const
    {
        return fileName(QDateTime::currentDateTime());
    }

    QString fileName(QDateTime dt) const
    {
        return QString("%1/%2_%3.%4")
                .arg(dir.absolutePath())
                .arg(sFileNameWithoutExt)
                .arg(dt.toString("hh"))
                .arg(sExt);
    }

    void log(const QByteArray & d){
        QFile f(fileName());
        if(mode == TRUNCATE)
        {
        removeDublicates(QFileInfo(f));
        } else if (mode == ZIP)
        {
            toArhive(QFileInfo(f));
        }

        if(f.open(QFile::Append))
        {
            Note note(d);
            f.write(note.toByteArray());

            /* обновляем время последней записи */
            lastWrite = QDateTime::currentDateTime();

            f.close();
        }
    }


    void removeDublicates(const QFileInfo& fi)
    {
        if(fi.exists())
        {
            QDateTime lastModified = fi.lastModified();
            bool remove =   lastModified.date().month() != QDate::currentDate().month()
                    ||  lastModified.date().day() != QDate::currentDate().day();

            if(remove)
                QFile(fi.absoluteFilePath()).remove();
        }
    }

    void toArhive(const QFileInfo &fi){
        QFileInfo nn(sFilePath);
        QString name = nn.baseName();
        QFileInfo oldFi;

        for(int hourOfDay = 24; hourOfDay > 0; hourOfDay--){
            int hour = 3600 * hourOfDay;
            QFileInfo tmp(fileName(QDateTime::currentDateTime().fromTime_t(time(nullptr) - hour)));
            if(tmp.isFile()){
                if(fi.lastModified().time().hour() == tmp.lastModified().time().hour()){
                    if(QDateTime::currentDateTime().date() !=  tmp.lastModified().date()){
                        oldFi = tmp;
                        break;
                    }
                    continue;
                }else {
                    oldFi = tmp;
                    break;
                }

            }
        }

        QString nameArchive = QString("%1_%2_%3").arg(QString::number(oldFi.lastModified().date().day()))
                .arg(QString::number(oldFi.lastModified().date().month())).arg(QString::number(oldFi.lastModified().date().year()));
        QString zipName = QString("%1").arg(QString("%1_%2.zip").arg(name).arg(nameArchive));

        if(oldFi.isFile()){            
            if (JlCompress::compressFile(zipName, oldFi.absoluteFilePath(), QuaZip::mdAdd))
            {
                QFile f(oldFi.absolutePath());
                f.remove(oldFi.absoluteFilePath());
                qDebug() << "zip opened";
                qDebug() << "added : " << oldFi.absoluteFilePath();
            }
        }
    }

};





#endif // QBINARYLOGGER_H
