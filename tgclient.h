#ifndef TGCLIENT_H
#define TGCLIENT_H

#include <QObject>
#include <QTcpSocket>

class TgTransport;

class TgClient : public QObject
{
    Q_OBJECT
private:
    TgTransport *_transport;

public:
    explicit TgClient(QObject *parent = 0);
    
signals:
    
public slots:
    void start();
    
};

#endif // TGCLIENT_H
