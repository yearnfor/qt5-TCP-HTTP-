#ifndef TRANSFERTASK_H
#define TRANSFERTASK_H

#include <QObject>
#include <QTimer>
#include <QtNetwork>
#include "transferconnection.h"

struct TaskInfo
{
    int              localPort;
    QString          type;
    QList<TargetInfo>targets;
};
class TransferTask : public QObject
{
    Q_OBJECT
public:
    explicit TransferTask(TaskInfo *& taskInfo, QObject *parent = nullptr);
    ~TransferTask();
    void startTask();
    void stopTask();
private:
    void stopAllClient();
public slots:
    void acceptConnection(); // 接收客户端连接
    void checkTask();
private:
    TaskInfo *taskInfo;
    QTcpServer *tcpServer;
    QTimer *checkTimer;
    QList<TransferConnection *>taskList;
};

#endif // TRANSFERTASK_H
