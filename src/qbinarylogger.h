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

class QBinaryLogger {

public:
    struct header {
        uint16_t m_mkr;
        uint64_t m_tm;
        uint16_t m_len;

        uint64_t time() const {
            return m_tm & ~((uint64_t)(0xffc0) << 48);
        }

        uint64_t ms() const {
            return m_tm >> 54;
        }

        uint16_t len() const {
            return m_len;
        }
    };



    QString sFilePath;
    QDir    dir;
    QString sFileNameWithoutExt;
    QString sExt;

    QDateTime lastWrite;
    QMap<int, QByteArray> preambs;

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

    void addPreabula(int key, const QByteArray &d){
        preambs.insert(key, d);
    }

    QByteArray preambula(int key) const{
        return preambs.value(key);
    }

    void log(const QByteArray & d){
        QFile f(fileName());
        removeDublicates(QFileInfo(f));

        if(f.open(QFile::Append))
        {
            QTextStream ts(&f);
            ts << d;
            ts.flush();
            /* обновляем время последней записи */
            lastWrite = QDateTime::currentDateTime();
            f.write(d);
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
