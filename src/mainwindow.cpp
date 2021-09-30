#include <QGraphicsView>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "headers/mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QAction *actionAbout = ui->menubar->addAction("About");
    graphicScene = new QGraphicsScene;
    ui->graphicsView->setScene(graphicScene);

    barsAmount = 1;
    ui->barAmountSpinBox->setValue(barsAmount);

    ui->barTableWidget->setColumnCount(4); // 4 - кол-во параметров стержней
    ui->barTableWidget->setRowCount(barsAmount);
    ui->barTableWidget->setHorizontalHeaderLabels(QStringList() << "L" << "A" << "[σ]" << "E");
    for(int i = 0; i < ui->barTableWidget->columnCount(); i++)
        ui->barTableWidget->setColumnWidth(i,62);

    ui->forceFTableWidget->setRowCount(barsAmount+1);
    ui->forceQTableWidget->setRowCount(barsAmount);

    for(int i = 0; i < ui->barTableWidget->rowCount(); i++){
        ui->barTableWidget->setRowHeight(i, 5);
        for(int j = 0; j < ui->barTableWidget->columnCount();j++){
            QTableWidgetItem *item = new QTableWidgetItem("");
            ui->barTableWidget->setItem(i,j, item);
        }
    }

    for(int i = 0; i < ui->forceFTableWidget->rowCount(); i++){
        ui->forceFTableWidget->setRowHeight(i, 5);
        QTableWidgetItem *item = new QTableWidgetItem("");
        ui->forceFTableWidget->setItem(i,1, item);
    }

    for(int i = 0; i < ui->forceQTableWidget->rowCount(); i++){
        ui->forceQTableWidget->setRowHeight(i, 5);
        QTableWidgetItem *item = new QTableWidgetItem("");
        ui->forceQTableWidget->setItem(i,1, item);
    }

    connect(actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::exit);
    connect(ui->barAmountSpinBox, &QSpinBox::valueChanged, this, &MainWindow::barAmountValueChanged);
    connect(ui->barTableWidget, &QTableWidget::itemChanged, this, &MainWindow::barTableCellValueChanged);
    connect(ui->forceFTableWidget, &QTableWidget::itemChanged, this, &MainWindow::forceTableCellValueChanged);
    connect(ui->forceQTableWidget, &QTableWidget::itemChanged, this, &MainWindow::forceTableCellValueChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete graphicScene;
}

void MainWindow::barAmountValueChanged(){
    barsAmount = ui->barAmountSpinBox->value();
    if(barsAmount > 8){ // 8 - максимальное кол-во стержней (так захотелось)
        barsAmount = 8;
        ui->barAmountSpinBox->setValue(barsAmount);
    }
    if(barsAmount > ui->barTableWidget->rowCount()){
        int previousValue = ui->barTableWidget->rowCount();

        ui->barTableWidget->setRowCount(barsAmount);
        for(int i = previousValue; i < barsAmount; i++)
            for(int j = 0; j < ui->barTableWidget->columnCount(); j++){
                QTableWidgetItem *item = new QTableWidgetItem("");
                ui->barTableWidget->setItem(previousValue, j, item);
            }

        ui->forceQTableWidget->setRowCount(barsAmount);
        ui->forceFTableWidget->setRowCount(barsAmount+1);

        for(int i = previousValue; i < barsAmount + 1; i++){
            QTableWidgetItem *item = new QTableWidgetItem("");
            ui->forceFTableWidget->setItem(i, 1, item);
        }

        for(int i = previousValue; i < barsAmount; i++){
            QTableWidgetItem *item = new QTableWidgetItem("");
            ui->forceQTableWidget->setItem(i, 1, item);
        }
    }
    else if (barsAmount < 1){ // 1 - минимальное кол-во стержней (а что -1 стержень может быть конструкцией?)
        barsAmount = 1;
        ui->barAmountSpinBox->setValue(barsAmount);
    }
    else{
        ui->barTableWidget->setRowCount(barsAmount);
        ui->forceQTableWidget->setRowCount(barsAmount);
        ui->forceFTableWidget->setRowCount(barsAmount + 1);
    }
}
// це костыль
void MainWindow::barTableCellValueChanged(QTableWidgetItem *item){
    QString cellValue = item->text();

    ui->barTableWidget->blockSignals(true);
    if(item->text().toDouble() == 0 || item->text().toDouble() < 0){
        item->setBackground(QBrush(Qt::red));
    }
    else
        item->setBackground(QBrush(Qt::white));
    ui->barTableWidget->blockSignals(false);
}

void MainWindow::forceTableCellValueChanged(QTableWidgetItem *item){
    QString cellValue = item->text();

    item->tableWidget()->blockSignals(true);
    if(item->text().toDouble() == 0){
        item->setBackground(QBrush(Qt::red));
    }
    else
        item->setBackground(QBrush(Qt::white));
    item->tableWidget()->blockSignals(false);
}

void MainWindow::about(){
    QMessageBox::about(this,tr("About application"),
                       tr("Тут могла быть ваша реклама"));
}

void MainWindow::exit(){
    QCoreApplication::exit();
}

