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

class QBinaryLogger {
public:
#pragma pack(1)
    struct header {
        uint16_t m_mkr;
        uint64_t m_tm;
        uint16_t m_len;

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

    struct Note {
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
                int r = ts.readRawData((char*)&n.h, sizeof (header));
                if(r != sizeof(header)) {
                    continue;
                }

                n.d = QByteArray(n.h.len(), 0);

                r = ts.readRawData(n.d.data(), n.d.size());

                if(r == n.h.len())
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

    QBinaryLogger(const QString &filename)
    {
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
        QFile(fileName()).remove();
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
        removeDublicates(QFileInfo(f));

        if(f.open(QFile::Append))
        {
            header h;

            uint64_t t =  t = QDateTime::currentDateTime().toTime_t() | ((uint64_t)(QTime::currentTime().msecsSinceStartOfDay() % 1000) << 54);
            h.m_tm =  qToLittleEndian<uint64_t>(t);
            h.m_mkr = qToLittleEndian<uint16_t>(0x7FF0);
            h.m_len = qToLittleEndian<uint16_t>(d.size());
            f.write(QByteArray((const char*) &h, sizeof (h)) + d);

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
};




#endif // QBINARYLOGGER_H
