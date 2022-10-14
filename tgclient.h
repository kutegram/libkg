#ifndef TGCLIENT_H
#define TGCLIENT_H

#include <QObject>
#include <QTcpSocket>

class TgTransport;

class TgClient : public QObject
{
    Q_OBJECT
private:
    QTcpSocket *_socket;
    TgTransport *_transport;

public:
    explicit TgClient(QObject *parent = 0);
    QTcpSocket* socket();
    
signals:
    
public slots:
    void start(QString ip, quint16 port);
    void _connected();
    
};

#endif // TGCLIENT_H
