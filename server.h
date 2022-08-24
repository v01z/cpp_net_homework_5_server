#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>
#include <QTime>
#include <QFile>
#include <QCoreApplication>

//------------------------------------------------------------------

class Server : public QObject
{
    Q_OBJECT
private:
    QTcpServer *tcpserver_;

    void sendToClient(QTcpSocket*, const QString&);

    const QString getRequestType(const QString&);
    const QString getTargetResource(const QString&);

    void handleGetRequest(QTcpSocket*, const QString&);
    void handlePostRequest(QTcpSocket*, const QString&);
    void handleWrongRequest(QTcpSocket*, const int,
        const QString&);



public:
    explicit Server(uint, QObject *parent = nullptr);
    Server() = delete;

public slots:
    void slotNewConnection();
    void slotReadClient();

signals:

};

//------------------------------------------------------------------

#endif // SERVER_H
