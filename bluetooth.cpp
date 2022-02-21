#include "mainwindow.h"
#include "ui_mainwindow.h"

bool MainWindow::InitBluetooth()
{
    bluetoothChat = new Chat(this);
    connect(bluetoothChat,&Chat::BluetoothReceiveAnswer,this,&MainWindow::GetBluetoothAnswer);
    bluetoothChat->show();
    Bluetoothtimer = new QTimer(this);
    Bluetoothtimer->setInterval(5000);
    Bluetoothtimer->setSingleShot(true);
}

// 蓝牙发送信息
void MainWindow::BluetoothSendMessage(QString message)
{
    BluetoothAnswer = ""; // 清空接受栏
    bluetoothChat->sendMessage(message);
    bluetoothChat->showMessage(bluetoothChat->localName,message);
}

// 发送发射参数信息给发射端
void MainWindow::SendTransmitParam()
{
    // 查看是否蓝牙连接
    if(IsBluetoothConnect())
    {
        QString param = QString("CSIParam frep:%1:time:%2:atenNum:%3:platform:%4:")\
                .arg(CurrentCollectFrequence,CurrentCollectSecond,CurrentAtenNumber,CurrentPlatformName);
        BluetoothSendMessage(param);
        if(CheckBluetoothAnswer(param,2,"params update"))
            ui->textBrowser_cmd->append("param Update success");
        else
        {
            BluetoothStatus = 0;
            RefreshOneLED(1);// 刷灯
        }
    }
}

// 获取蓝牙收到信息
void MainWindow::GetBluetoothAnswer(QString anwser)
{
    BluetoothAnswer = anwser;
    qDebug()<<"bluetooth get"<<BluetoothAnswer;
    if(BluetoothAnswer.contains("CSIParam"))// 包含参数信息
    {
        QStringList CSIParams;
        CSIParams = BluetoothAnswer.split(":");
        CurrentCollectFrequence= CSIParams[1];
        CurrentCollectSecond = CSIParams[3];
        CurrentAtenNumber = CSIParams[5];
        CurrentPlatformName = CSIParams[7];
        UpdateDisplayTraPC();
        BluetoothSendMessage("CSI params update!");
    }
    // 结束收集指令
    else if (BluetoothAnswer.contains(FinishCollectData)) {
        qDebug()<<"bluttooth 48";
        proc->close();
        PaperForNextTimeCollect();
    }
    // 开始收集指令
    else if (BluetoothAnswer.contains(StartCollectData)) {
        CollectDataTraPC();
    }
}

// 蓝牙是否连接
bool MainWindow::IsBluetoothConnect()
{
    BluetoothExpectAnswer = "connect"; //需要的回答
    BluetoothAnswer = "waiting";
    BluetoothSendMessage("Bluetooth connect test");

    switch (CheckBluetoothAnswer("Bluetooth connect test",5,"connect")) {
    case BLTCONNECT:
        BluetoothStatus = 1;
        RefreshOneLED(1);// 刷灯
        qDebug()<<"connect";
        return BLTCONNECT;
    case BLTDISCONNECT:
        BluetoothStatus = 0;
        RefreshOneLED(1);// 刷灯
        qDebug()<<"disconnect";
        return BLTDISCONNECT;
    }
}



// 检查蓝牙传入字符是否符合要求
bool MainWindow:: CheckBluetoothAnswer(QString Message,int tryingTime,QString WantAnswer)
{
    QEventLoop BluetoothEventLoop;
    int currentTime = 0;
    BluetoothAnswer = "";
    for(;currentTime<tryingTime;currentTime++)
    {
        QTimer::singleShot(5000, &BluetoothEventLoop, SLOT(quit()));
        BluetoothEventLoop.exec();
        qDebug()<<"bluetooth_answer   "<< BluetoothAnswer<<"WantAnswer   "<<WantAnswer;
        if(BluetoothAnswer.contains(WantAnswer))
        {
            return BLTCONNECT;
        }
    qDebug()<<"it is"<<currentTime<<"time";
    BluetoothSendMessage(Message);
    }
    return BLTDISCONNECT;
}
