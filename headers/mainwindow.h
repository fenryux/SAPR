#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
//#include "headers/bargraphicitem.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void barAmountValueChanged();
    void barTableCellValueChanged(QTableWidgetItem *item);
    void forceTableCellValueChanged(QTableWidgetItem *item);
    void leftSupportValueChanged(const int& state);
    void rightSupportValueChanged(const int& state);
    void clearBarData();
    void draw();
//    void saveProjectFile();
    void saveAs();
//    void open();
    void about();
    void exit();

private:
    bool isRowValid(QList<QTableWidgetItem *> barData);
    bool isBarTableValid();
    bool isForceTableValid(const QTableWidget* table);

    Ui::MainWindow *ui;
    QString currentFile;

    int barsAmount;

    QGraphicsScene* graphicScene;
    QGraphicsLineItem* lineItem;
    QGraphicsPixmapItem* leftSupport;
    QGraphicsPixmapItem* rightSupport;

    QList<QList<QTableWidgetItem*>> barsList;
    QList<QTableWidgetItem*> forceFList;
    QList<QTableWidgetItem*> forceQList;

};
#endif // MAINWINDOW_H
