#include "headers/mainwindow.h"
#include "ui_mainwindow.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QAction *actionAbout = ui->menubar->addAction("About");

    graphicScene = new QGraphicsScene;
    ui->graphicsView->setScene(graphicScene);
//    leftSupport = new QGraphicsPixmapItem(QPixmap(":/resources/images/leftSupport.png"));
//    rightSupport = new QGraphicsPixmapItem(QPixmap(":/resources/images/rightSupport.png"));
//    leftSupport->hide();
//    rightSupport->hide();

    barsAmount = 0;
    ui->barAmountSpinBox->setValue(barsAmount);

    ui->barTableWidget->setColumnCount(4); // 4 - кол-во параметров стержней
    ui->barTableWidget->setRowCount(barsAmount);
    ui->barTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->forceFTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->forceQTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->forceFTableWidget->setRowCount(barsAmount);
    ui->forceQTableWidget->setRowCount(barsAmount);

    connect(actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::exit);
    connect(ui->barAmountSpinBox, &QSpinBox::valueChanged, this, &MainWindow::barAmountValueChanged);
    connect(ui->barTableWidget, &QTableWidget::itemChanged, this, &MainWindow::barTableCellValueChanged);
    connect(ui->forceFTableWidget, &QTableWidget::itemChanged, this, &MainWindow::forceTableCellValueChanged);
    connect(ui->forceQTableWidget, &QTableWidget::itemChanged, this, &MainWindow::forceTableCellValueChanged);
    connect(ui->clearBarTablePushButton, &QPushButton::pressed, this, &MainWindow::clearBarData);
    connect(ui->sealingLeftCheckBox, &QCheckBox::stateChanged, this, &MainWindow::leftSupportValueChanged);
    connect(ui->sealingRightCheckBox, &QCheckBox::stateChanged, this, &MainWindow::rightSupportValueChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete graphicScene;
    delete leftSupport;
    delete rightSupport;
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
        ui->forceQTableWidget->setRowCount(barsAmount);
        ui->forceFTableWidget->setRowCount(barsAmount + 1);

        for(int i = previousValue; i < ui->forceFTableWidget->rowCount(); i++){
            QTableWidgetItem *item = new QTableWidgetItem;
            item->setTextAlignment(Qt::AlignCenter);
            item->setBackground(QBrush(Qt::red));
            ui->forceFTableWidget->setItem(i, 0, item);
        }

        for(int i = previousValue; i < barsAmount; i++){
            QTableWidgetItem *item = new QTableWidgetItem;
            item->setTextAlignment(Qt::AlignCenter);
            item->setBackground(QBrush(Qt::red));
            ui->forceQTableWidget->setItem(i, 0, item);
        }
    }
    else if (barsAmount < 0){
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
            ui->barTableWidget->item(i,j)->setText("");
            ui->barTableWidget->item(i,j)->setBackground(QBrush(Qt::red));
        }
    graphicScene->clear();
    ui->barTableWidget->blockSignals(false);
}

void MainWindow::barTableCellValueChanged(QTableWidgetItem *item){
    QRegularExpression regExp("^[1-9]+[0-9]*$");
    QRegularExpressionMatch match = regExp.match(item->text());

    ui->barTableWidget->blockSignals(true);
    if(match.hasMatch())
        item->setBackground(QBrush(Qt::white));
    else
        item->setBackground(QBrush(Qt::red));
    ui->barTableWidget->blockSignals(false);

    QList<QTableWidgetItem *> barData;
    for(int i = 0; i < 4; i++)
        barData.append(ui->barTableWidget->item(item->row(), i));

    if(isRowValid(barData)){
        if(!barsList.contains(barData))
            barsList.replace(barData.at(0)->row(), barData);
        if(isTableValid()) // если данные о ВСЕХ стержнях валидны, то происходит отрисовка
            draw();
    }
}

void MainWindow::forceTableCellValueChanged(QTableWidgetItem *item){
    QRegularExpression regExp("^[1-9]+[0-9]*$");
    QRegularExpressionMatch match = regExp.match(item->text());

    item->tableWidget()->blockSignals(true);
    if(match.hasMatch())
        item->setBackground(QBrush(Qt::white));
    else
        item->setBackground(QBrush(Qt::red));
    item->tableWidget()->blockSignals(false);
}

bool MainWindow::isRowValid(QList<QTableWidgetItem *> barData){
    for(auto i: barData){
        if(i->background() == QBrush(Qt::red))
            return false;
    }
    return true;
}

bool MainWindow::isTableValid(){
    for(int i = 0; i < ui->barTableWidget->rowCount(); i++){
        for(int j = 0; j < ui->barTableWidget->columnCount(); j++){
//            qDebug() << ui->barTableWidget->item(i,j)->text() << ' ' << ui->barTableWidget->item(i,j)->text().toDouble();
            if(ui->barTableWidget->item(i,j)->background().color() == Qt::red)
                return false;
        }
    }
    return true;
}
// P.S. Отрисовка происходит с учетом того факта, что все данные о стержнях валидны
void MainWindow::draw(){
    graphicScene->clear();

    QList<QGraphicsRectItem*> rects;
    leftSupport = graphicScene->addPixmap(QPixmap(":/resources/images/leftSupport.png"));
    rightSupport = graphicScene->addPixmap(QPixmap(":/resources/images/rightSupport.png"));
    leftSupport->hide();
    rightSupport->hide();

    QGraphicsRectItem* rectItem = graphicScene->addRect(0,0,barsList.at(0).at(0)->text().toDouble()*50,
                                                        barsList.at(0).at(1)->text().toDouble()*25, QPen(Qt::black,5));
    rects.append(rectItem);

    lineItem = graphicScene->addLine(-1000, rectItem->rect().center().ry(), 2000, rectItem->rect().center().ry(),QPen(Qt::DashDotLine));
    lineItem->setFlags(QGraphicsItem::ItemStacksBehindParent);

    leftSupport->setPos((0 - leftSupport->pixmap().width()),
                        rectItem->rect().center().ry()-leftSupport->pixmap().height()/2);

    if(barsList.begin()+1 != barsList.end())
    for(auto i = barsList.begin()+1; i < barsList.end(); i++){
        double x = rects.at(rects.size()-1)->rect().topRight().rx();
        double dy = ((*i).at(1)->text().toDouble() - (i-1)->at(1)->text().toDouble())*25/2;
        double y = rects.at(rects.size()-1)->rect().topRight().ry() - dy;

        QGraphicsRectItem* rectItem = graphicScene->addRect(x,y,(*i).at(0)->text().toDouble()*50,
                                                            (*i).at(1)->text().toDouble()*25, QPen(Qt::black,5));
        rects.append(rectItem);
    }
    rightSupport->setPos(rects.at(rects.size()-1)->rect().topRight().rx(),
                        leftSupport->y());

    if(ui->sealingLeftCheckBox->isChecked())
        leftSupport->show();
    if(ui->sealingRightCheckBox->isChecked())
        rightSupport->show();
}

void MainWindow::leftSupportValueChanged(const int& state){
    if(barsAmount == 0 || !isTableValid())
        return;
    if(state != 2)
        leftSupport->hide();
    else leftSupport->show();
}

void MainWindow::rightSupportValueChanged(const int& state){
    if(barsAmount == 0 || !isTableValid())
        return;
    if(state != 2)
        rightSupport->hide();
    else rightSupport->show();
}

void MainWindow::about(){
    QMessageBox::about(this,tr("About application"),
                       tr("Тут могла быть ваша реклама"));
}

void MainWindow::exit(){
    QCoreApplication::exit();
}

