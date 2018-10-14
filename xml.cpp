#include "xml.h"

Xml::Xml(QString name)
    :fileName(name)
{

}

Xml::~Xml()
{

}

bool Xml::init(bool read, QString rootNodeName)
{
    QFile file(fileName);

    qDebug() << "fileName" << fileName;
    if(read){
        if(file.exists() != true){
            return false;
        }

        if(!file.open(QFile::ReadOnly)){
            qDebug() << "open" << fileName << "error";
            return false;
        }
        if(!doc.setContent(&file)){
            qDebug() << "set doc content error";
            file.close();
            return false;
        }
        file.close();
    }else{
        //xml header
        doc.appendChild(doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\""));
        //root node
        root = doc.createElement(rootNodeName);
        doc.appendChild(root);
    }

    return true;
}

bool Xml::subNode(QString name, QMap<QString, QString>& data)
{
    QDomElement root=doc.documentElement();
    QDomNode node = root.firstChild();

    while(!node.isNull()){
        if(node.isElement()){
            QDomElement element = node.toElement();
            if(element.tagName() == name){
                QDomNodeList list = element.childNodes();
                for(int i = 0; i < list.count(); i++){
                    QDomNode n = list.at(i);
                    if(n.isElement()){
                        //qDebug() << n.nodeName() << n.toElement().text();
                        data.insert(n.nodeName(), n.toElement().text());
                    }
                }
                if(data.isEmpty() == false){
                    return true;
                }
                return false;
            }
        }
        node = node.nextSibling();
    }

    return false;
}

bool Xml::addSubNode(QString name, QMap<QString, QString> &data)
{
    QMap<QString, QString>::const_iterator map;
    QDomElement node=doc.createElement(name);
    QDomElement subNode;

    for(map=data.begin(); map!=data.end(); map++){
        subNode = doc.createElement(map.key());
        subNode.appendChild(doc.createTextNode(map.value()));
        node.appendChild(subNode);
    }
    root.appendChild(node);

    return true;
}

bool Xml::save()
{
    QFile file(fileName);

    if(!file.open(QFile::WriteOnly|QFile::Truncate)){
        qDebug() << "open" << fileName << "error";
        return false;
    }

    QTextStream outStream(&file);
    doc.save(outStream, 4);
    file.close();

    return true;
}

bool Xml::isEmpty()
{
    return doc.isNull();
}

void Xml::clear()
{
    doc.clear();
}
