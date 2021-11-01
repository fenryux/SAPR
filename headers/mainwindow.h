#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>

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
    void draw();
    void calculate();
    void saveAs();
    void open();
    void about();
    void exit();

protected:
#if QT_CONFIG(wheelevent)
    void wheelEvent(QWheelEvent* event) override;
#endif
    void scaleView(qreal scaleFactor);

private:
    bool isRowValid(QList<QTableWidgetItem *> barData);
    bool isBarTableValid();
    bool isForceTableValid(const QTableWidget* table);
    void clearDataTables();
    QList<double> Gauss(QList<QList<double>> &matrixA, QList<double> &matrixB);

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
    QList<QList<double>> resultAList;
    QList<double> resultBList;
    QList<double> resultDeltaList;
    QList<QList<double>> resultNXList;

};
#endif // MAINWINDOW_H
