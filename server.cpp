#include "server.h"

//------------------------------------------------------------------

Server::Server(uint port, QObject *parent)
    : QObject{ parent },
    tcpserver_{}
{
   tcpserver_ = new QTcpServer(this);
   if(!tcpserver_->listen(QHostAddress::Any, port))
   {
       qInfo() << "Error: Unable to start server: "
        << tcpserver_->errorString();

       tcpserver_->close();
       return;
   }
    connect(tcpserver_, SIGNAL(newConnection()), this,
        SLOT(slotNewConnection()));

   qInfo() << "Server started at " << tcpserver_->serverAddress().toString()
        << ":" << tcpserver_->serverPort();

}

//------------------------------------------------------------------

void Server::slotNewConnection() {
    QTcpSocket* client_socket = tcpserver_->nextPendingConnection();

    connect(client_socket, SIGNAL(disconnected()), client_socket,
            SLOT(deleteLater()));
    connect(client_socket, SIGNAL(readyRead()), this,
            SLOT(slotReadClient()));

    sendToClient(client_socket, "Server Response:  Connected! "
                                "Issue smth like: \'GET /path/filename\'");
}

//------------------------------------------------------------------

void Server::slotReadClient() {
    QTcpSocket* client_socket = (QTcpSocket*)sender();

    QDataStream in_stream(client_socket);
    in_stream.setVersion(QDataStream::Qt_5_15);

    while(true)
    {

        if(client_socket->bytesAvailable() == 0)
        {
            break;
        }

        QString str;
        in_stream >> str;

        qInfo() << "Got from client "
                << client_socket->peerAddress().toString()
                << client_socket->peerPort()
                << ":\n" << str << "\n";

        QString request_type { getRequestType(str) };

        if (request_type == "GET")
            handleGetRequest(client_socket,
                getTargetResource(str));

        else if(request_type == "POST")
            handlePostRequest(client_socket,
                getTargetResource(str));

        else
            handleWrongRequest(client_socket, 400, "Bad Request");
    }
}

//------------------------------------------------------------------

void Server::sendToClient(QTcpSocket* client_socket, const QString& str) {
   QByteArray arr_block = str.toUtf8();

   client_socket->write(arr_block);
}

//------------------------------------------------------------------

const QString Server::getRequestType(const QString& request) {
    return QString(request.section(' ', 0, 0));
}

//------------------------------------------------------------------

const QString Server::getTargetResource(const QString& request) {
    return QString(request.section(' ', 1, 1));
}

//------------------------------------------------------------------
void Server::handleGetRequest(QTcpSocket* client_sock,
        const QString& requested_resource) {

    QString resource{ requested_resource };



    qInfo() << "Client " << client_sock->peerAddress() << ":" << client_sock->peerPort()
            << " asked for a file: " << resource;

    QFile file(resource);
    if (!file.open(QIODevice::ReadOnly)) {
        qInfo() << "File " << requested_resource << " not exists or cannot be opened.";
        handleWrongRequest(client_sock, 404, "Not Found");
        return;
    }

    quint64 file_size = file.size();
    QByteArray file_block = file.readAll();

    QString header { "HTTP/1.1 200 OK\r\nContent-Length: " +
                     QString::number(file_size) + "\r\n\r\n" };


    QByteArray response_block = header.toUtf8();
    response_block.append(file_block);

    qInfo() << "Sending file..";
    client_sock->write(response_block);

    qInfo() << "Sent: ";
    qInfo() << "Header size: " << header.size();
    qInfo() << "Resp_block size: " << response_block.size();
    qInfo() << "File size: " << file.size();
}

void Server::handlePostRequest(QTcpSocket* client_sock,
    const QString& requested_command) {

    if(requested_command == "/control/exit") {
        qInfo() << "POST exit detected";

        sendToClient(client_sock, "Server message: \'Disconnect\'");
        client_sock->waitForBytesWritten();
        client_sock->deleteLater();
        QCoreApplication::quit();
    }

}

//------------------------------------------------------------------

void Server::handleWrongRequest(QTcpSocket* client_sock,
        const int error_code, const QString& description ) {

    QString response{ "HTTP/1.1 " + QString::number(error_code)+ " " +
        description + "\r\nContent-Length: " +
            QString::number(description.length()) + "\r\n\r\n" + description };

    sendToClient(client_sock, response);
}

//------------------------------------------------------------------
