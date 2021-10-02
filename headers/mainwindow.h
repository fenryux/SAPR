#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include "headers/bargraphicitem.h"

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
    void clearBarData();
    void draw(const QList<QTableWidgetItem *> barData);
//    void saveProjectFile();
//    void saveAs();
//    void open();
    void about();
    void exit();


private:
    bool isValid(QList<QTableWidgetItem *> barData);

    Ui::MainWindow *ui;
    QString currentFile;

    int barsAmount;

    QGraphicsScene* graphicScene;
    QList<BarGraphicItem> barGraphicList;
    QGraphicsLineItem* lineItem;

    QList<QList<QTableWidgetItem*>> barsList;

};
#endif // MAINWINDOW_H
