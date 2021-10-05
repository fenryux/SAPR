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

    barsAmount = 0;
    ui->barAmountSpinBox->setValue(barsAmount);

    ui->barTableWidget->setColumnCount(4); // 4 - кол-во параметров стержней
    ui->barTableWidget->setRowCount(barsAmount);
    ui->barTableWidget->setHorizontalHeaderLabels(QStringList() << "L, м" << "A, м^2" << "[σ], кН" << "E, МПа");
    for(int i = 0; i < ui->barTableWidget->columnCount(); i++)
        ui->barTableWidget->setColumnWidth(i,62);

    ui->forceFTableWidget->setRowCount(barsAmount+1);
    ui->forceQTableWidget->setRowCount(barsAmount);

    connect(actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::exit);
    connect(ui->barAmountSpinBox, &QSpinBox::valueChanged, this, &MainWindow::barAmountValueChanged);
    connect(ui->barTableWidget, &QTableWidget::itemChanged, this, &MainWindow::barTableCellValueChanged);
    connect(ui->forceFTableWidget, &QTableWidget::itemChanged, this, &MainWindow::forceTableCellValueChanged);
    connect(ui->forceQTableWidget, &QTableWidget::itemChanged, this, &MainWindow::forceTableCellValueChanged);
    connect(ui->clearBarTablePushButton, &QPushButton::pressed, this, &MainWindow::clearBarData);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete graphicScene;
}

void MainWindow::barAmountValueChanged(){
    barsAmount = ui->barAmountSpinBox->value();

    ui->barTableWidget->blockSignals(true);
    ui->forceFTableWidget->blockSignals(true);
    ui->forceQTableWidget->blockSignals(true);

    if(barsAmount > ui->barTableWidget->rowCount()){
        int previousValue = ui->barTableWidget->rowCount();

        ui->barTableWidget->setRowCount(barsAmount);
        for(int i = previousValue; i < barsAmount; i++)
            for(int j = 0; j < ui->barTableWidget->columnCount(); j++){
                QTableWidgetItem *item = new QTableWidgetItem;
                item->setTextAlignment(Qt::AlignCenter);
                item->setBackground(QBrush(Qt::red));
                ui->barTableWidget->setItem(previousValue, j, item);
            }

        barsList.resize(barsAmount);
//        barGraphicList.reserve(barsAmount);
        ui->forceQTableWidget->setRowCount(barsAmount);
        ui->forceFTableWidget->setRowCount(barsAmount+1);

        for(int i = previousValue; i < barsAmount + 1; i++){
            QTableWidgetItem *item = new QTableWidgetItem;
            item->setTextAlignment(Qt::AlignCenter);
            item->setBackground(QBrush(Qt::red));
            ui->forceFTableWidget->setItem(i, 1, item);
        }

        for(int i = previousValue; i < barsAmount; i++){
            QTableWidgetItem *item = new QTableWidgetItem;
            item->setTextAlignment(Qt::AlignCenter);
            item->setBackground(QBrush(Qt::red));
            ui->forceQTableWidget->setItem(i, 1, item);
        }
    }
    else if (barsAmount < 0){ // 1 - минимальное кол-во стержней (а что -1 стержень может быть конструкцией?)
        barsAmount = 0;
        ui->barAmountSpinBox->setValue(barsAmount);
    }
    else{
        ui->barTableWidget->setRowCount(barsAmount);
        ui->forceQTableWidget->setRowCount(barsAmount);
        ui->forceFTableWidget->setRowCount(barsAmount + 1);
        barsList.resize(barsAmount);
//        barGraphicList.resize(barsAmount);
    }
    ui->barTableWidget->blockSignals(false);
    ui->forceFTableWidget->blockSignals(false);
    ui->forceQTableWidget->blockSignals(false);
}

void MainWindow::clearBarData(){
    ui->barTableWidget->blockSignals(true);
    for(int i = 0; i < ui->barTableWidget->rowCount(); i++)
        for(int j = 0; j < ui->barTableWidget->columnCount(); j++){
            ui->barTableWidget->item(i,j)->setText("");
            ui->barTableWidget->item(i,j)->setBackground(QBrush(Qt::red));
        }
    ui->barTableWidget->blockSignals(false);
}
// це костыль
void MainWindow::barTableCellValueChanged(QTableWidgetItem *item){
//    QString cellValue = item->text();

    ui->barTableWidget->blockSignals(true);
    if(item->text().toDouble() == 0 || item->text().toDouble() < 0){
        item->setBackground(QBrush(Qt::red));
    }
    else
        item->setBackground(QBrush(Qt::white));
    ui->barTableWidget->blockSignals(false);

    QList<QTableWidgetItem *> barData;
    for(int i = 0; i < 4; i++)
        barData.append(ui->barTableWidget->item(item->row(), i));

    if(isRowValid(barData)){
        if(!barsList.contains(barData))
            barsList.replace(barData.at(0)->row(), barData);
        if(isTableValid())
            draw();
    }
}

void MainWindow::forceTableCellValueChanged(QTableWidgetItem *item){
//    QString cellValue = item->text();

    item->tableWidget()->blockSignals(true);
    if(item->text().toDouble() == 0){
        item->setBackground(QBrush(Qt::red));
    }
    else
        item->setBackground(QBrush(Qt::white));
    item->tableWidget()->blockSignals(false);
}

bool MainWindow::isRowValid(QList<QTableWidgetItem *> barData){
    for(auto i: barData){
        if(i->background() == QBrush(Qt::red) || i->text() == "")
            return false;
    }
    return true;
}

bool MainWindow::isTableValid(){
    for(int i = 0; i < ui->barTableWidget->rowCount(); i++){
        for(int j = 0; j < ui->barTableWidget->columnCount(); j++){
//            qDebug() << ui->barTableWidget->item(i,j)->text() << ' ' << ui->barTableWidget->item(i,j)->text().toDouble();
            if(ui->barTableWidget->item(i,j)->text().toDouble() == 0)
                return false;
        }
    }
    return true;
}

void MainWindow::draw(){
    QList<QGraphicsRectItem*> rects;
    graphicScene->clear();

    QGraphicsRectItem* rectItem = graphicScene->addRect(0,0,barsList.at(0).at(0)->text().toDouble()*50,
                                                        barsList.at(0).at(1)->text().toDouble()*25, QPen(Qt::black,5));
    rects.append(rectItem);

    lineItem = graphicScene->addLine(-1000, rectItem->rect().center().ry(), 2000, rectItem->rect().center().ry(),QPen(Qt::DashDotLine));
    lineItem->setFlags(QGraphicsItem::ItemStacksBehindParent);
    if(barsList.begin()+1 != barsList.end())
    for(auto i = barsList.begin()+1; i < barsList.end(); i++){
        double x = rects.at(rects.size()-1)->rect().topRight().rx();
//        auto tmp = (i-1)->at(1)->text().toDouble();
        double dy = ((*i).at(1)->text().toDouble() - (i-1)->at(1)->text().toDouble())*25/2;
        double y = rects.at(rects.size()-1)->rect().topRight().ry() - dy;
        QGraphicsRectItem* rectItem = graphicScene->addRect(x,y,(*i).at(0)->text().toDouble()*50,
                                                            (*i).at(1)->text().toDouble()*25, QPen(Qt::black,5));
        rects.append(rectItem);
    }
}

void MainWindow::about(){
    QMessageBox::about(this,tr("About application"),
                       tr("Тут могла быть ваша реклама"));
}

void MainWindow::exit(){
    QCoreApplication::exit();
}

