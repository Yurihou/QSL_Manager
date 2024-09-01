#include <QString>

QString stringXOR(QString str, QString key)
{
    if(key.isEmpty())key = "114514";
    while(key.length() < str.length())key.append(key);
    if(key.length() > str.length())key = key.left(str.length());
    QByteArray bs1 = str.toLatin1();
    QByteArray bs2 = key.toLatin1();
    for(int i=0; i<str.length(); i++)
    {
        bs1[i] = bs1[i] ^ bs2[i];
    }

    QString result;
    return result.prepend(bs1);
}
