#ifndef TRANSFERCONNECTION_H
#define TRANSFERCONNECTION_H
#include <QObject>
#include <QtNetwork>
struct TargetInfo
{
    QString targetIp;
    int     targetPort;
    QString type;
};
class TransferConnection : public QObject
{
    Q_OBJECT
public:
    TransferConnection(QTcpSocket *& client, QString& type, QList<TargetInfo>& targets, QObject *parent = nullptr);
    ~TransferConnection();
    bool isValid();

    void stop();
public slots:
    void receiveClientData();
    void receiveServerData();
    void procConnectErr(QAbstractSocket::SocketError);
    void diconnect();
private slots:
    void connectTargetTcpSocket();
private:
    QString type;
    QList<TargetInfo>targets;

    QTcpSocket *clientSocket;
    QList<QTcpSocket *>targetSockets;
    QTcpSocket *targetTcpSocket;
    QTcpSocket *targetHttpSocket;
    int connectCount = 0;
    bool socketBuild = false;
    bool isHttp = false;
};

#endif // TRANSFERCONNECTION_H
