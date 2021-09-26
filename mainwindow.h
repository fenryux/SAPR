#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>

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
//    void dataChanged();
//    void validate();
//    void draw();
//    void goToProcessor();
//    void goToPostprocessor();
//    void saveProjectFile();
//    void saveAs();
//    void open();
    void about();
    void exit();


private:
    Ui::MainWindow *ui;
    QString currentFile;
    QGraphicsScene preprocessorScene;

};
#endif // MAINWINDOW_H
