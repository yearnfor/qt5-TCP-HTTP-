#include "mainservice.h"

MainService::MainService(QObject *parent) : QObject(parent)
{}

void MainService::initial()
{
    QFile   configFile;
    QString appPath = qApp->applicationDirPath();
    configFile.setFileName(appPath + "/tcpTransferConfig.json");
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Can't open the file!" << configFile;
        return;
    }
    QByteArray configData = configFile.readAll();
    QJsonParseError jsonError;
    QJsonDocument   json = QJsonDocument::fromJson(configData, &jsonError);
    if (jsonError.error != QJsonParseError::NoError)
    {
        qDebug() << jsonError.errorString();
        return;
    }
    QJsonObject rootObj = json.object();
    if (!rootObj.contains("tasks"))
    {
        qDebug() << "tasks missing";
        return;
    }
    else
    {
        QJsonArray tastArray = rootObj.value("tasks").toArray();
        for (int i = 0; i < tastArray.size(); i++)
        {
            QJsonObject task = tastArray.at(i).toObject();
            TaskInfo    taskInfo;
            if (!task.contains("localPort"))
                return;
            taskInfo.localPort = task.value("localPort").toInt();
            if (!task.contains("type"))
                return;
            taskInfo.type = task.value("type").toString();
            if (!task.contains("targets"))
                return;
            QJsonArray targetArray = task.value("targets").toArray();
            for (int j = 0; j < targetArray.size(); j++)
            {
                QJsonObject target = targetArray.at(j).toObject();
                TargetInfo  ip_port_type;
                if (!target.contains("targetIp"))
                    return;
                ip_port_type.targetIp = target.value("targetIp").toString();
                if (!target.contains("targetPort"))
                    return;
                ip_port_type.targetPort = target.value("targetPort").toInt();
                if (!target.contains("type"))
                    return;
                ip_port_type.type = target.value("type").toString();
                taskInfo.targets.append(ip_port_type);
            }
            taskInfoList.append(taskInfo);
        }
    }
    qDebug() << QString::fromLocal8Bit("获取配置成功");
}

void MainService::start()
{
    for (int i = 0; i < taskInfoList.size(); i++)
    {
        TaskInfo *info = &taskInfoList[i];
        TransferTask *task = new TransferTask(info, this);
        taskList.append(task);
    }
    for (int i = 0; i < taskList.size(); i++)
    {
        taskList.at(i)->startTask();
    }
}
