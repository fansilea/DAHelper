#ifndef XML_H
#define XML_H

#include <QtXml>
#include <QString>

class Xml
{
public:
    Xml(QString name);
    ~Xml();

    bool init(bool read=true, QString rootNodeName="root");
    bool addSubNode(QString name, QMap<QString, QString>& data);
    bool subNode(QString name, QMap<QString, QString>& data);
    bool isEmpty();
    bool save();
    bool close();    
    void clear();

private:
    QString fileName;
    QString rootName;
    QDomDocument doc;
    QDomElement root;
};

#endif // XML_H
