#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/icon/icon.ico"));
    //  QProcess *usbip_process;
    appPath = QCoreApplication::applicationDirPath() + USBIP_PATH;

    usbip_process
        = new QProcess(this);
    usbip_list_pro = new QProcess(this);
    connect(usbip_process, &QProcess::readyReadStandardOutput, this,
        &MainWindow::task);
    //    startTimer(5000);
    listtask();
}

MainWindow::~MainWindow() { delete ui; }

// 连接按钮回调
void MainWindow::on_connect_button_clicked()
{
    // 判断输入框中是否有ip地址
    int timeoutMilliseconds = 5000; // 设置超时时间为5000毫秒（5秒）
    if (ui->lineEdit_ip->text().isNull()) {
        qDebug() << "请输入IP";
        ui->log_view->append(QString("请输入IP"));
    } else {
        QString ip = ui->lineEdit_ip->text();
        //        QString Cmd = USBIP_PATH + "attach -r " + ip.replace("\"", " ") + " -b 1-1";
        QString Cmd = QString("%1 attach -r %2 -b 1-1 ").arg(appPath).arg(ip);
        usbip_process->start(Cmd);
        qDebug() << Cmd;
    }

    //    if (!usbip_process->waitForStarted()) {
    //        qDebug() << "Failed to start the external program.";
    //    }
    if (!usbip_process->waitForFinished(timeoutMilliseconds)) {
        qDebug() << "Process timed out or error occurred.";
        ui->log_view->append(QString("连接超时，请检查你的ip是否正确！"));
        usbip_process->kill();
    }
}
// 断开按钮回调
void MainWindow::on_disconnect_button_clicked()
{

    QItemSelectionModel* selectionModel = ui->devices_list->selectionModel();
    // 获取所有选中项的索引
    QModelIndexList indexes = selectionModel->selectedIndexes();

    // 遍历所有选中项的索引
    for (int i = 0; i < indexes.size(); ++i) {
        // 获取当前选中项的模型索引
        QModelIndex index = indexes.at(i);
        // 获取当前选中项的数据，这里假设我们获取显示角色的数据
        QString itemData = ui->devices_list->model()->data(index, Qt::DisplayRole).toString();
        // 现在你可以使用itemData变量，它包含了选中项的数据
        qDebug() << "Selected item:" << itemData;
        QRegularExpression re("port\\s(\\d+)"); // 正则表达式，\\s 表示空白字符，\\d+ 表示一个或多个数字
        QRegularExpressionMatch match = re.match(itemData);
        if (match.hasMatch()) { // 如果找到匹配
            int portNumber = match.captured(1).toInt(); // 提取第一个括号内匹配的数字
            qDebug() << "Port number:" << portNumber;
            // 获取到port后，将其detach
            QString Cmd = QString("%1 detach -p %2").arg(appPath).arg(portNumber);
            usbip_process->start(Cmd);
            qDebug() << Cmd;
            if (!usbip_process->waitForStarted()) {
                qDebug() << "Failed to start the external program.";
            }
        } else {
            qDebug() << "No port number found.";
        }
    }
}
// 任务处理
void MainWindow::task()
{
    QByteArray output = usbip_process->readAllStandardOutput();
    QString log = QTextCodec::codecForName("GBK")->toUnicode(output);
    // 将输出添加到log框
    ui->log_view->append(log);
    //
    //
    if (log.contains("succesfully attached")) {
        QRegularExpression re("port\\s(\\d+)"); // 正则表达式，\\s 表示空白字符，\\d+ 表示一个或多个数字
        QRegularExpressionMatch match = re.match(log);
        if (match.hasMatch()) { // 如果找到匹配
            int portNumber = match.captured(1).toInt(); // 提取第一个括号内匹配的数字
            attach_list_t temp;
            temp.ip = ui->lineEdit_ip->text();
            temp.attact_port = portNumber;
            attach_list.append(temp);
            addElem2list(temp);
            qDebug() << "Port number:" << portNumber;
        } else {
            qDebug() << "No port number found.";
        }

    } else if (log.contains("succesfully detached")) {
        QItemSelectionModel* selectionModel = ui->devices_list->selectionModel();

        // 获取所有选中项的索引
        QModelIndexList indexes = selectionModel->selectedIndexes();

        // 从最后一项开始删除，避免索引变化导致的问题
        for (int i = indexes.size() - 1; i >= 0; --i) {
            QModelIndex index = indexes.at(i);
            // 从模型中删除该项
            model->removeRow(index.row(), index.parent());
        }
    } else {
        qDebug() << "执行出错";
    }
    listtask();
}

void MainWindow::addElem2list(attach_list_t t)
{
    // 添加条目
    model = new QStandardItemModel;

    // 创建QStandardItem对象并添加到模型中
    QString item_string = QString("%1               attached in port %2").arg(t.ip).arg(t.attact_port);
    QStandardItem* item = new QStandardItem(item_string);
    model->appendRow(item);
    ui->devices_list->setModel(model);
}

// void MainWindow::timerEvent(QTimerEvent* t)
void MainWindow::listtask()
{

    qDebug() << "start list task";
    QString Cmd = QString("%1 list -l").arg(appPath);
    usbip_list_pro->start(Cmd);
    if (!usbip_list_pro->waitForStarted()) {
        qDebug() << "usblist task start fail";
    } else {
        if (usbip_list_pro->waitForFinished()) {
            QByteArray output = usbip_list_pro->readAllStandardOutput();
            QString log = QTextCodec::codecForName("GBK")->toUnicode(output);
            qDebug() << log;
            // 将输出按行分割
            QStringList lines = log.split("\n", QString::SkipEmptyParts);
            QStandardItemModel* model = new QStandardItemModel; // 创建模型
            QStandardItem *leftItem, *rightItem;

            for (int i = 0; i < lines.size(); i += 2) {
                // 获取busid行和设备信息行
                QString busidLine = lines.at(i);
                QString deviceLine = (i + 1 < lines.size()) ? lines.at(i + 1) : "";

                // 解析busid
                QRegExp busidRegex("busid\\s+(\\S+)");
                if (busidRegex.indexIn(busidLine) != -1) {
                    QString busid = busidRegex.cap(1);
                    leftItem = new QStandardItem(busid);
                } else {
                    leftItem = new QStandardItem(busidLine);
                }

                // 解析设备信息
                QRegExp deviceInfoRegex("(.*)\\s+:\\s+(.*)");
                if (deviceInfoRegex.indexIn(deviceLine) != -1) {
                    QString vendor = deviceInfoRegex.cap(1);
                    QString product = deviceInfoRegex.cap(2);
                    rightItem = new QStandardItem(vendor + " : " + product);
                } else {
                    rightItem = new QStandardItem(deviceLine);
                }

                // 添加到模型
                model->setItem(i / 2, 0, leftItem);
                model->setItem(i / 2, 1, rightItem);

                // 设置table

                ui->list_view->setWordWrap(false);
                ui->list_view->resizeColumnsToContents();
                ui->list_view->setModel(model);
                //                ui->list_view->show();
            }
        }
    }
}
