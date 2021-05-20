#include "transfertask.h"

TransferTask::TransferTask(TaskInfo *& taskInfo, QObject *parent)
    : taskInfo(taskInfo), QObject(parent)
{
    tcpServer = new QTcpServer(this);
    checkTimer = new QTimer(this);
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
    connect(checkTimer, SIGNAL(timeout()), this, SLOT(checkTask()));
}

TransferTask::~TransferTask()
{
    stopAllClient();
    checkTimer->stop();
    tcpServer->close();
}

void TransferTask::startTask()
{
    if (!tcpServer->listen(QHostAddress::Any, taskInfo->localPort))
    {
        tcpServer->close();
        qDebug() << QString("正在监听:127.0.0.1:%1").arg(taskInfo->localPort);
    }
    checkTimer->start(10000);
}

void TransferTask::stopTask()
{}

void TransferTask::stopAllClient()
{
    QList<TransferConnection *>::iterator iter;
    for (iter = taskList.begin(); iter != taskList.end(); iter++)
    {
        (*iter)->stop();
        delete (*iter);
    }
    taskList.clear();
}

void TransferTask::acceptConnection()
{
    QTcpSocket *tcpSocket;
    tcpSocket = tcpServer->nextPendingConnection();
    TransferConnection *newConnection = new TransferConnection(tcpSocket, taskInfo->type, taskInfo->targets);
    taskList.append(newConnection);
}

void TransferTask::checkTask()
{
    QList<TransferConnection *>::iterator iter;
    for (iter = taskList.begin(); iter != taskList.end();)
    {
        if ((*iter)->isValid())
        {
            iter++;
            continue;
        }
        else
        {
            delete (*iter);
            taskList.erase(iter++);
        }
    }
}
