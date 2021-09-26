#include <QGraphicsView>
#include <QFileDialog>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QAction *actionAbout = ui->menubar->addAction("About");

    barsAmount = 1;
    ui->barAmountSpinBox->setValue(barsAmount);

    ui->barTableWidget->setColumnCount(4); // 4 - кол-во параметров стержней
    ui->barTableWidget->setRowCount(barsAmount);
    ui->barTableWidget->setHorizontalHeaderLabels(QStringList() << "L" << "A" << "sigma" << "E");
    for(int i = 0; i < 4; i++)
        ui->barTableWidget->setColumnWidth(i,62);

    ui->forceTableWidget->setColumnCount(2); // 2 - продольные и сосредоточенные нагрузки
    ui->forceTableWidget->setRowCount(barsAmount+1);
    ui->forceTableWidget->setHorizontalHeaderLabels(QStringList() << "F" << "q");
    for(int i = 0; i < 2; i++)
        ui->forceTableWidget->setColumnWidth(i,25);

    for(int i = 0; i < ui->barTableWidget->rowCount(); i++){
        ui->barTableWidget->setRowHeight(i, 5);
        for(int j = 0; j < 4;j++){
            QTableWidgetItem *item = new QTableWidgetItem("");
            ui->barTableWidget->setItem(i,j, item);
        }
    }
    // обдумать момент с кол-вом ячеек для сосредоточенных нагрузок (их кол-во = кол-во стержней + 1)
    for(int i = 0; i < ui->forceTableWidget->rowCount(); i++){
        ui->forceTableWidget->setRowHeight(i, 5);
        for(int j = 0; j < 2;j++){
            QTableWidgetItem *item = new QTableWidgetItem("");
            if(j != 2)
                ui->forceTableWidget->setItem(i,j, item);
        }
    }

    connect(actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::exit);
    connect(ui->barAmountSpinBox, &QSpinBox::valueChanged, this, &MainWindow::barAmountValueChanged);
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
            for(int j = 0; j < 4; j++){
                QTableWidgetItem *item = new QTableWidgetItem("");
                ui->barTableWidget->setItem(previousValue, j, item);
            }
        // обдумать момент с кол-вом ячеек для сосредоточенных нагрузок (их кол-во = кол-во стержней + 1)
        ui->forceTableWidget->setRowCount(barsAmount);
        for(int i = previousValue; i < barsAmount; i++)
            for(int j = 0; j < 2; j++){
                QTableWidgetItem *item = new QTableWidgetItem("");
                if(j != 2)
                    ui->forceTableWidget->setItem(previousValue, j, item);
            }

    }
    else if (barsAmount < 1){ // 1 - минимальное кол-во стержней (а что -1 стержень может быть конструкцией?)
        barsAmount = 1;
        ui->barAmountSpinBox->setValue(barsAmount);
    }
    else{
        ui->barTableWidget->setRowCount(barsAmount);
        ui->forceTableWidget->setRowCount(barsAmount);
    }
    // добавить изменения в таблице нагрузок
}

void MainWindow::about(){
    QMessageBox::about(this,tr("About application"),
                       tr("Placeholder text"));
}

void MainWindow::exit(){
    QCoreApplication::exit();
}

MainWindow::~MainWindow()
{
    delete ui;
}

