#include <QCoreApplication>
#include <QBinaryLogger>

QString hex(uchar bt)
{
    return QString("%1").arg(bt,2,16,QChar('0')).toUpper();
}

QString hex(const void* const ptr, size_t len, char spacer = ' ')
{
    const char * const data = (char*) ptr;
    QString tmp;
    for (size_t i = 0; i < len; i++)
    {
        tmp += hex(data[i]) + spacer;
    }
    return tmp.trimmed();
}

QString hex(const QByteArray & array, char spacer = ' ')
{
    return hex(array.constData(), array.length(), spacer);
}

QString hex(const QString& str, char spacer)
{
    return hex(str.toLocal8Bit(), spacer);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QBinaryLogger b("test.txt");
    b.log(QByteArray(5, 0xAA));


    QFile  f(b.fileName());
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    qDebug() << hex(f.readAll());

    f.seek(0);
    QBinaryLogger::header h;
    f.read((char*)&h, sizeof h);

    qDebug() << "Длина записи:" << h.len();
    qDebug() << "Время записи:" << QString("%1::%2").arg(QDateTime::fromTime_t(h.time()).toString()).arg(h.ms());

    auto notes = QBinaryLogger::read(b.fileName());

    for(auto n : notes)
    {
        qDebug() << hex(&n.h, sizeof n.h) << hex(n.d);
    }

    f.close();

    return a.exec();
}

