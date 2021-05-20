#ifndef MAINSERVICE_H
#define MAINSERVICE_H

#include <QObject>
#include "transfertask.h"
class MainService : public QObject
{
    Q_OBJECT
public:
    explicit MainService(QObject *parent = nullptr);
    void initial();
    void start();
signals:
private:
    QList<TaskInfo>taskInfoList;
    QList<TransferTask *>taskList;
};

#endif // MAINSERVICE_H
