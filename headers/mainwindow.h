#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
//    bool isValid();
private slots:
    void barAmountValueChanged();
    void barTableCellValueChanged(QTableWidgetItem *item);
    void forceTableCellValueChanged(QTableWidgetItem *item);
//    void validate();
//    void draw();
//    void saveProjectFile();
//    void saveAs();
//    void open();
    void about();
    void exit();


private:
    Ui::MainWindow *ui;
    QString currentFile;
    int barsAmount;
    QGraphicsScene* graphicScene;

};
#endif // MAINWINDOW_H
