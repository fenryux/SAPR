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

    connect(actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::exit);
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

