#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QTextCodec>
#include <qdebug.h>

#include <QStandardItem>
#include <QStandardItemModel>
#include <QTimer>
#include <QVector>

#define USBIP_PATH "./usbip-win/usbip.exe"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    typedef struct {
        QString ip;
        int attact_port;
    } attach_list_t;

    //    void timerEvent(QTimerEvent* t);
    void listtask();

    void task();

    void addElem2list(attach_list_t t);

private slots:
    void on_connect_button_clicked();
    void on_disconnect_button_clicked();

private:
    Ui::MainWindow* ui;
    QProcess* usbip_process;
    QProcess* usbip_list_pro;
    QVector<attach_list_t> attach_list;
    QStandardItemModel* model;
    QString appPath;
};
#endif // MAINWINDOW_H
