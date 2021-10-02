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
                QTableWidgetItem *item = new QTableWidgetItem("1");
                item->setTextAlignment(Qt::AlignCenter);
                ui->barTableWidget->setItem(previousValue, j, item);
            }

        barsList.resize(barsAmount);
        ui->forceQTableWidget->setRowCount(barsAmount);
        ui->forceFTableWidget->setRowCount(barsAmount+1);

        for(int i = previousValue; i < barsAmount + 1; i++){
            QTableWidgetItem *item = new QTableWidgetItem("");
            item->setTextAlignment(Qt::AlignCenter);
            ui->forceFTableWidget->setItem(i, 1, item);
        }

        for(int i = previousValue; i < barsAmount; i++){
            QTableWidgetItem *item = new QTableWidgetItem("");
            item->setTextAlignment(Qt::AlignCenter);
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
    }
    ui->barTableWidget->blockSignals(false);
    ui->forceFTableWidget->blockSignals(false);
    ui->forceQTableWidget->blockSignals(false);
}

void MainWindow::clearBarData(){
    ui->barTableWidget->blockSignals(true);
    for(int i = 0; i < ui->barTableWidget->rowCount(); i++)
        for(int j = 0; j < ui->barTableWidget->columnCount(); j++){
            ui->barTableWidget->item(i,j)->setText("1");
            ui->barTableWidget->item(i,j)->setBackground(QBrush(Qt::white));
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

    if(isValid(barData)){
        if(!barsList.contains(barData))
            barsList.replace(barData.at(0)->row(), barData);
        draw(barData);
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

bool MainWindow::isValid(QList<QTableWidgetItem *> barData){
    for(auto i: barData){
        if(i->background() == QBrush(Qt::red) || i->text() == "")
            return false;
    }
    return true;
}

void MainWindow::draw(const QList<QTableWidgetItem *> barData){
    if(barGraphicList.isEmpty()){
        QGraphicsRectItem* rectItem = graphicScene->addRect(0,0,barData.at(0)->text().toDouble()*50,
                                                            barData.at(1)->text().toDouble()*25, QPen(Qt::black, 5));
        lineItem = graphicScene->addLine(-1000, rectItem->rect().center().ry(), 2000,
                                         rectItem->rect().center().ry(), QPen(Qt::DashDotLine));
        lineItem->setFlags(QGraphicsItem::ItemStacksBehindParent);
        barGraphicList.append(BarGraphicItem(rectItem, barData.at(0)->text().toDouble(),
                                             barData.at(1)->text().toDouble()));
    }
    else{
        double x = barsList.at(barsList.indexOf(barData) - 1).at(0)->text().toDouble() * 50;
        double y = barsList.at(barsList.indexOf(barData) - 1).at(1)->text().toDouble() * 25;

        QGraphicsRectItem* rectItem = graphicScene->addRect(x,y,barData.at(0)->text().toDouble()*50,
                                                            barData.at(1)->text().toDouble()*25, QPen(Qt::black, 5));
        barGraphicList.append(BarGraphicItem(rectItem, barData.at(0)->text().toDouble(),
                                             barData.at(1)->text().toDouble()));
    }
}

void MainWindow::about(){
    QMessageBox::about(this,tr("About application"),
                       tr("Тут могла быть ваша реклама"));
}

void MainWindow::exit(){
    QCoreApplication::exit();
}

