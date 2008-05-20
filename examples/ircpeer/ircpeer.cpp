#include "ircpeer.h"
#include <QRegExp>

IRCPeer::IRCPeer(QObject* parent) : QxtRPCPeer(parent) 
{
    qRegisterMetaType<IRCName>();
}

bool IRCPeer::canDeserialize(const QByteArray& buffer) const 
{
    return buffer.indexOf('\n') != -1 || buffer.indexOf('\r') != -1;
}

static bool serializeParam(QTextStream& str, QVariant p) 
{
    if(!p.isValid() || !p.canConvert(QVariant::ByteArray)) 
    {
        str << "\n\r" << flush;
        return true;
    }
    QByteArray ba = p.toByteArray();
    if(ba.contains(' ')) 
    {
        str << " :" << ba << "\n\r" << flush;
        return true;
    }
    str << " " << ba << flush;
    return false;
}

QByteArray IRCPeer::serialize(QString fn, QVariant p1, QVariant p2, QVariant p3, QVariant p4, QVariant p5, QVariant p6, QVariant p7, QVariant p8, QVariant p9) const 
    {
    QByteArray rv;
    QTextStream str(&rv);
    if(p1.isValid()) {
        if(p1.canConvert<IRCName>())
            str << ":" << p1.value<IRCName>().assemble() << " ";
        else if(!p1.toByteArray().isEmpty())
            str << ":" << p1.toByteArray() << " ";
    }
    str << fn << flush;

    if(serializeParam(str, p2)) return rv;
    if(serializeParam(str, p3)) return rv;
    if(serializeParam(str, p4)) return rv;
    if(serializeParam(str, p5)) return rv;
    if(serializeParam(str, p6)) return rv;
    if(serializeParam(str, p7)) return rv;
    if(serializeParam(str, p8)) return rv;
    if(serializeParam(str, p9)) return rv;
    return rv + "\n\r";
}

QPair<QString, QList<QVariant> > IRCPeer::deserialize(QByteArray& data) 
    {
    int lfPos = data.indexOf('\n'), crPos = data.indexOf('\r'), wordPos = 0, endPos = (crPos == -1 || lfPos < crPos) ? lfPos : crPos;
    QByteArray message = data.left(endPos).trimmed(), prefix;
    data = data.mid(endPos+1);
    if(message.isEmpty()) return qMakePair(QString(), QList<QVariant>());
        qDebug() << message;

    QList<QByteArray> words = message.split(' ');
    if(words[0][0] == ':') {
        prefix = words[0].mid(1);
        while(wordPos < words.count() && words[wordPos++].isEmpty()) 
            return qMakePair(QString(), QList<QVariant>() << QVariant());
    }

    QString command(words[wordPos]);
    QList<QVariant> params;
    if(command.toInt() != 0) {
        params << QVariant::fromValue(IRCName((prefix + ":" + command).toUtf8(), "", prefix));
        command = "numeric";
    } else {
        if(prefix.isEmpty())          params << QVariant();
        else if(prefix.contains('@')) params << QVariant::fromValue(IRCName::fromName(prefix));
        else                          params << prefix;
    }

    while(wordPos++ < words.count()-1) {
        if(words[wordPos].isEmpty()) continue;
        if(words[wordPos][0] == ':') {
            QByteArray trailing = words[wordPos++].mid(1);
            while(wordPos < words.count()) 
                trailing = trailing + ' ' + words[wordPos++];
            params << trailing;
            break;
        }
        params << words[wordPos];
    }

    if(command == "numeric") {
        params.removeAt(1);
        while(params[1].toByteArray() == "=" || params[1].toByteArray() == "@") params.removeAt(1);
        if(params[1].toByteArray()[0] != '#') {
            command = "system";
            params.push_front(QByteArray());
            params.swap(0, 1);
        }
    }
    
    return qMakePair(command, params);
}

IRCName IRCName::fromName(const QByteArray& name) 
{
    QRegExp re("([^@!]*)(?:!([^@]*)?)(?:@(.*))?");
    re.exactMatch(QString::fromUtf8(name.data()));
    return IRCName(re.cap(1).toUtf8(), re.cap(2).toUtf8(), re.cap(3).toUtf8());
}

IRCName::IRCName(QByteArray nick, QByteArray ident, QByteArray host) : nick(nick), ident(ident), host(host) {}

QByteArray IRCName::assemble() const 
{
    return nick + (ident.isEmpty() ? QByteArray() : "!" + ident) + (host.isEmpty() ? QByteArray() : "@" + host);
}
