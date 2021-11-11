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
    void ppShowTableView();
//    void ppShowGraphicView(int arg1);
    void draw();
    void calculate();
    void saveAs();
    void open();
    void about();
    void exit();

    void on_NXCheckBox_stateChanged(int arg1);
    void on_UXCheckBox_stateChanged(int arg1);
    void on_SigmaCheckBox_stateChanged(int arg1);

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
    void configurePostprocessor();
    void drawPostprocessor();

    Ui::MainWindow *ui;
    QString currentFile;

    int barsAmount;
    // графические объекты препроцессора
    QGraphicsScene* graphicScene;
    QGraphicsScene* postprocessorGraphicScene;
    QGraphicsLineItem* lineItem;
    QGraphicsPixmapItem* leftSupport;
    QGraphicsPixmapItem* rightSupport;
    // данные для препроцессора и процессора
    QList<QList<QTableWidgetItem*>> barsList;
    QList<QTableWidgetItem*> forceFList;
    QList<QTableWidgetItem*> forceQList;
    // конечные данные для постпроцессора - результат работы процессора
    QList<QList<double>> resultAList;
    QList<double> resultBList;
    QList<double> resultDeltaList;
    QList<QList<double>> resultNXList;
    QList<QList<double>> resultUXList;
    QList<QList<double>> resultSigmaXList;
    // графические объекты постпроцессора
    QList<QGraphicsLineItem*> NXGraphicItems;
    QList<QGraphicsLineItem*> UXGraphicItems;
    QList<QGraphicsLineItem*> SigmaGraphicItems;
};
#endif // MAINWINDOW_H
