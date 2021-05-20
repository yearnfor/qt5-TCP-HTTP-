#include "transferconnection.h"

TransferConnection::TransferConnection(QTcpSocket *& client, QString& type, QList<TargetInfo>& targets, QObject *parent)
    : clientSocket(client), type(type), targets(targets), QObject(parent)
{
    qDebug() << QString::fromLocal8Bit("新连接:");
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(receiveClientData()));
    connect(clientSocket, SIGNAL(disconnected()), this, SLOT(diconnect()));
    if (this->type == "group")
    {
        for (int i = 0; i < targets.size(); i++)
        {
            QTcpSocket *targetSocket = new QTcpSocket(this);
            targetSockets.append(targetSocket);
            connect(targetSocket, SIGNAL(readyRead()), this, SLOT(receiveServerData()));
            connect(targetSocket, SIGNAL(disconnected()), this, SLOT(diconnect()));
            connect(targetSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                    this, SLOT(procConnectErr(QAbstractSocket::SocketError)));
            targetSocket->connectToHost(this->targets.at(i).targetIp, this->targets.at(i).targetPort);
            if (targetSocket->waitForConnected(4000))
            {
                connectCount++;
                qDebug() << targetSocket->localAddress().toString() << ":" << targetSocket->localPort() << ">>"
                         << targetSocket->peerAddress().toString() << ":" << targetSocket->peerPort();
            }
        }
    }
    else
    {
        QTimer::singleShot(100, this, SLOT(connectTargetTcpSocket()));
        targetHttpSocket = new QTcpSocket(this);
        connect(targetHttpSocket, SIGNAL(readyRead()), this, SLOT(receiveServerData()));
        connect(targetHttpSocket, SIGNAL(disconnected()), this, SLOT(diconnect()));
        connect(targetHttpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(procConnectErr(QAbstractSocket::SocketError)));
    }
}

TransferConnection::~TransferConnection()
{
    disconnect(clientSocket, SIGNAL(disconnected()), this, SLOT(diconnect()));
    clientSocket->close();
    for (int i = 0; i < targetSockets.size(); i++)
    {
        if (targetSockets.at(i)->state() == QTcpSocket::SocketState::ConnectedState)
        {
            disconnect(targetSockets[i], SIGNAL(disconnected()), this, SLOT(diconnect()));
            targetSockets.at(i)->close();

            // targetSockets.at(i)->waitForDisconnected(1000);
        }
    }
    targetSockets.clear();
}

bool TransferConnection::isValid()
{
    if (connectCount == 0)
    {
        disconnect(clientSocket, SIGNAL(disconnected()), this, SLOT(diconnect()));
        if (clientSocket->state() == QTcpSocket::SocketState::ConnectedState)
            clientSocket->close();
        return false;
    }
    else
        return true;
}

void TransferConnection::stop()
{
    disconnect(clientSocket, SIGNAL(disconnected()), this, SLOT(diconnect()));
    if (clientSocket->state() == QTcpSocket::SocketState::ConnectedState)
        clientSocket->close();

    for (int i = 0; i < targetSockets.size(); i++)
    {
        if (targetSockets.at(i)->state() == QTcpSocket::SocketState::ConnectedState)
        {
            disconnect(targetSockets[i], SIGNAL(disconnected()), this, SLOT(diconnect()));
            targetSockets.at(i)->close();
        }
    }
}

void TransferConnection::receiveClientData()
{
    if (this->type == "group")
    {
        while (clientSocket->bytesAvailable() > 0)
        {
            QByteArray datagram;
            datagram.resize(clientSocket->bytesAvailable());
            clientSocket->read(datagram.data(), datagram.size());
            for (int i = 0; i < targetSockets.size(); i++)
            {
                if (targetSockets.at(i)->state() == QTcpSocket::SocketState::ConnectedState)
                    targetSockets.at(i)->write(datagram);
            }
        }
    }
    else
    {
        while (clientSocket->bytesAvailable() > 0)
        {
            QByteArray datagram;
            datagram.resize(clientSocket->bytesAvailable());
            clientSocket->read(datagram.data(), datagram.size());
            if (!socketBuild)
            {
                socketBuild = true;
                if (QString(datagram).split('\n').first().contains("HTTP"))
                {
                    isHttp = true;
                    for (int i = 0; i < targets.size(); i++)
                    {
                        if (targets.at(i).type == "http")
                        {
                            targetHttpSocket->connectToHost(targets.at(i).targetIp, targets.at(i).targetPort);
                            if (targetHttpSocket->waitForConnected(4000))
                            {
                                connectCount++;
                                qDebug() << targetHttpSocket->localAddress().toString() << ":" << targetHttpSocket->localPort() << ">>"
                                         << targetHttpSocket->peerAddress().toString() << ":" << targetHttpSocket->peerPort();
                            }
                            break;
                        }
                    }
                }
            }

            if (isHttp)
                targetHttpSocket->write(datagram);
            else
                targetTcpSocket->write(datagram);
        }
    }
}

void TransferConnection::receiveServerData()
{
    QTcpSocket *targetSocket = qobject_cast<QTcpSocket *>(sender());
    if (isHttp && (targetSocket == targetTcpSocket))
        return;
    while (targetSocket->bytesAvailable() > 0)
    {
        QByteArray datagram;
        datagram.resize(targetSocket->bytesAvailable());
        targetSocket->read(datagram.data(), datagram.size());
        clientSocket->write(datagram);
    }
}

void TransferConnection::procConnectErr(QAbstractSocket::SocketError)
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    qDebug() << socket->errorString();
}

void TransferConnection::diconnect()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    qDebug() << QString::fromLocal8Bit("连接关闭:") << socket->localAddress().toString() << ":" << socket->localPort() << ">>"
             << socket->peerAddress().toString() << ":" << socket->peerPort();
    if (socket == clientSocket)
    {
        for (int i = 0; i < targetSockets.size(); i++)
        {
            if (targetSockets.at(i)->state() == QTcpSocket::SocketState::ConnectedState)
            {
                disconnect(targetSockets[i], SIGNAL(disconnected()), this, SLOT(diconnect()));
                targetSockets.at(i)->close();
            }
        }
        if (targetTcpSocket->state() == QTcpSocket::SocketState::ConnectedState)
        {
            disconnect(targetTcpSocket, SIGNAL(disconnected()), this, SLOT(diconnect()));
            targetTcpSocket->close();
        }
        if (targetHttpSocket->state() == QTcpSocket::SocketState::ConnectedState)
        {
            disconnect(targetHttpSocket, SIGNAL(disconnected()), this, SLOT(diconnect()));
            targetHttpSocket->close();
        }
    }
    else
    {
        connectCount--;
        if ((connectCount == 0) || (socket == targetHttpSocket))
        {
            disconnect(clientSocket, SIGNAL(disconnected()), this, SLOT(diconnect()));
            if (clientSocket->state() == QTcpSocket::SocketState::ConnectedState)
                clientSocket->close();
        }
    }
}

void TransferConnection::connectTargetTcpSocket()
{
    for (int i = 0; i < targets.size(); i++)
    {
        if (targets.at(i).type == "tcp")
        {
            targetTcpSocket = new QTcpSocket(this);
            connect(targetTcpSocket, SIGNAL(readyRead()), this, SLOT(receiveServerData()));
            connect(targetTcpSocket, SIGNAL(disconnected()), this, SLOT(diconnect()));
            connect(targetTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                    this, SLOT(procConnectErr(QAbstractSocket::SocketError)));
            targetTcpSocket->connectToHost(targets.at(i).targetIp, targets.at(i).targetPort);
            if (targetTcpSocket->waitForConnected(4000))
            {
                connectCount++;
                qDebug() << targetTcpSocket->localAddress().toString() << ":" << targetTcpSocket->localPort() << ">>"
                         << targetTcpSocket->peerAddress().toString() << ":" << targetTcpSocket->peerPort();
            }
            break;
        }
    }
}
