#include <QCoreApplication>
#include <QBinaryLogger>

QString hex(uchar bt)
{
    return QString("%1").arg(bt,2,16,QChar('0')).toUpper();
}

QString hex(const void* const ptr, size_t len, char spacer)
{
    const char * const data = (char*) ptr;
    QString tmp;
    for (size_t i = 0; i < len; i++)
    {
        tmp += hex(data[i]) + spacer;
    }
    return tmp.trimmed();
}

QString hex(const QByteArray & array, char spacer)
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
    QFile  testf("test_12.txt");
    testf.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray bytearray = testf.readAll();
    QBinaryLogger b("test.txt");

    b.log(bytearray);

    QFile  f("test_13.txt");
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    QString str = hex(f.readAll(), ' ');
    QStringList list = str.split(' ');
    QBinaryLogger::header h;
    b.log(f.readAll());
    f.read((char*)&h, sizeof(h));


     qDebug() << h.m_mkr << endl;
     qDebug() << h.time() << endl;
     qDebug() << h.len() << endl;

    f.close();

    return a.exec();
}

