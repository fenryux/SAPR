#include "headers/mainwindow.h"
#include "ui_mainwindow.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>

#define HEIGHT 45
#define WIDTH 60

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QAction *actionAbout = ui->menubar->addAction("About");

    graphicScene = new QGraphicsScene;
    ui->graphicsView->setScene(graphicScene);
    scale = 1;

    barsAmount = 1;
    ui->barAmountSpinBox->setValue(barsAmount);
    barsList.resize(barsAmount);
    forceFList.resize(barsAmount+1);
    forceQList.resize(barsAmount);

    ui->barTableWidget->setRowCount(barsAmount);
    ui->barTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->forceFTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->forceQTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //заполнение начальных данных по стержням
    QList<QTableWidgetItem*> barData;
    for(int i=0; i < 4; i++){
        QTableWidgetItem* item = new QTableWidgetItem("1");
        item->setBackground(QBrush(Qt::white));
        item->setTextAlignment(Qt::AlignCenter);
        ui->barTableWidget->setItem(0,i,item);
        barData.append(item);
    }
    barsList.replace(0,barData);

    ui->forceFTableWidget->setRowCount(barsAmount+1);
    ui->forceQTableWidget->setRowCount(barsAmount);
    //заполнение начальных данных по нагрузкам
    for(int i=0;i<barsAmount+1;i++){
        QTableWidgetItem* item = new QTableWidgetItem("0");
        item->setTextAlignment(Qt::AlignCenter);
        item->setBackground(Qt::white);
        ui->forceFTableWidget->setItem(i,0,item);
        forceFList.replace(i,item);
    }
    QTableWidgetItem* item = new QTableWidgetItem("0");
    item->setTextAlignment(Qt::AlignCenter);
    item->setBackground(Qt::white);
    ui->forceQTableWidget->setItem(0,0,item);
    forceQList.replace(0,item);

    draw();

    connect(actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::exit);
    connect(ui->actionSave_as, &QAction::triggered, this, &MainWindow::saveAs);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::open);
    connect(ui->barAmountSpinBox, &QSpinBox::valueChanged, this, &MainWindow::barAmountValueChanged);
    connect(ui->barTableWidget, &QTableWidget::itemChanged, this, &MainWindow::barTableCellValueChanged);
    connect(ui->forceFTableWidget, &QTableWidget::itemChanged, this, &MainWindow::forceTableCellValueChanged);
    connect(ui->forceQTableWidget, &QTableWidget::itemChanged, this, &MainWindow::forceTableCellValueChanged);
    connect(ui->sealingLeftCheckBox, &QCheckBox::stateChanged, this, &MainWindow::leftSupportValueChanged);
    connect(ui->sealingRightCheckBox, &QCheckBox::stateChanged, this, &MainWindow::rightSupportValueChanged);
    connect(ui->scaleMinusPushButton, &QPushButton::pressed, this, &MainWindow::scaleDecreasePressed);
    connect(ui->scalePlusPushButton, &QPushButton::pressed, this, &MainWindow::scaleIncreasePressed);
    connect(ui->scaleResetPushButton, &QPushButton::pressed, this, &MainWindow::scaleResetPressed);
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

    ui->barAmountSpinBox->blockSignals(true);
    ui->barTableWidget->blockSignals(true);
    ui->forceFTableWidget->blockSignals(true);
    ui->forceQTableWidget->blockSignals(true);

    if(barsAmount > ui->barTableWidget->rowCount()){
        int previousValue = ui->barTableWidget->rowCount();

        ui->barTableWidget->setRowCount(barsAmount);

        barsList.resize(barsAmount);
        forceQList.resize(barsAmount);
        forceFList.resize(barsAmount+1);

        for(int i = previousValue; i < ui->barTableWidget->rowCount(); i++){
            QList<QTableWidgetItem *> barData;
            for(int j = 0; j < ui->barTableWidget->columnCount(); j++){
                QTableWidgetItem *item = new QTableWidgetItem("1");

                item->setTextAlignment(Qt::AlignCenter);
                item->setBackground(QBrush(Qt::white));

                ui->barTableWidget->setItem(i, j, item);
                barData.append(item);
            }
            if(!barsList.contains(barData))
               barsList.replace(barData.at(0)->row(), barData);
        }
        if(isBarTableValid()) // если данные о ВСЕХ стержнях валидны, то происходит отрисовка
           draw();
        ui->forceQTableWidget->setRowCount(barsAmount);
        ui->forceFTableWidget->setRowCount(barsAmount + 1);

        for(int i = previousValue; i < ui->forceFTableWidget->rowCount(); i++){
            QTableWidgetItem *item = new QTableWidgetItem("0");

            item->setTextAlignment(Qt::AlignCenter);
            item->setBackground(Qt::white);

            if(i == ui->forceFTableWidget->rowCount()-1){
                if(ui->sealingRightCheckBox->isChecked()){
                    ui->forceFTableWidget->setItem(i,0,item);
                    item->setFlags(Qt::NoItemFlags);
                }
                else{
                    ui->forceFTableWidget->setItem(i, 0, item);
                }
            }
            else{
                ui->forceFTableWidget->setItem(i, 0, item);
            }
            forceFList.replace(i,item);
        }

        for(int i = previousValue; i < barsAmount; i++){
            QTableWidgetItem *item = new QTableWidgetItem("0");
            item->setTextAlignment(Qt::AlignCenter);
            item->setBackground(Qt::white);
            ui->forceQTableWidget->setItem(i, 0, item);
        }

        if(isBarTableValid())
            draw();
    }
    else if (barsAmount < 1){
        barsAmount = 1;
        ui->barAmountSpinBox->setValue(barsAmount);
    }
    else{
        ui->barTableWidget->setRowCount(barsAmount);
        ui->forceQTableWidget->setRowCount(barsAmount);
        ui->forceFTableWidget->setRowCount(barsAmount + 1);
        if(ui->sealingRightCheckBox->isChecked()){
            ui->forceFTableWidget->item(barsAmount,0)->setText("0");
            ui->forceFTableWidget->item(barsAmount,0)->setFlags(Qt::NoItemFlags);
            forceFList.replace(barsAmount, ui->forceFTableWidget->item(barsAmount,0));
        }
        barsList.resize(barsAmount);
        forceFList.resize(barsAmount+1);
        forceQList.resize(barsAmount);

        if(isBarTableValid())
            draw();
    }
    ui->barAmountSpinBox->blockSignals(false);
    ui->barTableWidget->blockSignals(false);
    ui->forceFTableWidget->blockSignals(false);
    ui->forceQTableWidget->blockSignals(false);
}

void MainWindow::barTableCellValueChanged(QTableWidgetItem *item){
    QRegularExpression regExp("(^[1-9]+[0-9]*$)|(^[1-9]+\\.?[0-9]*$)|(^[0-9]+\\.{1}[0-9]*$)");
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
        if(isBarTableValid()) // если данные о ВСЕХ стержнях валидны, то происходит отрисовка
            draw();
    }
}

void MainWindow::forceTableCellValueChanged(QTableWidgetItem *item){
    QRegularExpression regExp("^[0-9]$|(^-?[1-9]+[0-9]*$)");
    QRegularExpressionMatch match = regExp.match(item->text());

    item->tableWidget()->blockSignals(true);
    if(match.hasMatch()){
        item->setBackground(QBrush(Qt::white));
        if(item->tableWidget()->rowCount() == barsAmount + 1){
            if(!forceFList.contains(item))
                forceFList.replace(item->row(), item);
        }
        else
            if(!forceQList.contains(item))
                forceQList.replace(item->row(), item);
        if(isForceTableValid(item->tableWidget()) && isBarTableValid())
            draw();
    }
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

bool MainWindow::isBarTableValid(){
    for(int i = 0; i < ui->barTableWidget->rowCount(); i++){
        for(int j = 0; j < ui->barTableWidget->columnCount(); j++){
//            qDebug() << ui->barTableWidget->item(i,j)->text() << ' ' << ui->barTableWidget->item(i,j)->text().toDouble();
            if(ui->barTableWidget->item(i,j)->background().color() == Qt::red)
                return false;
        }
    }
    return true;
}

bool MainWindow::isForceTableValid(const QTableWidget* table){
    for(int i = 0; i < table->rowCount(); i++)
        if(table->itemAt(i,0)->background().color() == Qt::red)
            return false;
    return true;
}

void MainWindow::calculate(){
    if(!isBarTableValid() || !isForceTableValid(ui->forceFTableWidget) || !isForceTableValid(ui->forceQTableWidget)){
        QMessageBox::warning(this,"Warning!","Некорректные данные!");
        return;
    }

    QList<QList<double>> AList;
    QList<double> bList;
    QList<double> deltaList;


}
// P.S. Отрисовка происходит с учетом того факта, что все данные о стержнях валидны
void MainWindow::draw(){
    graphicScene->clear();

    QList<QGraphicsRectItem*> rects;
    QList<QGraphicsPixmapItem*> forcesF;

    leftSupport = graphicScene->addPixmap(QPixmap(":/resources/images/leftSupport.png"));
    rightSupport = graphicScene->addPixmap(QPixmap(":/resources/images/rightSupport.png"));
    leftSupport->hide();
    rightSupport->hide();

    // отрисовка 1-го стержня
    QGraphicsRectItem* rectItem = graphicScene->addRect(0,0,barsList.at(0).at(0)->text().toDouble()*WIDTH*scale,
                                                        barsList.at(0).at(1)->text().toDouble()*HEIGHT*scale, QPen(Qt::black,3));
    rects.append(rectItem);
    if(forceQList.at(0) != nullptr)
        if(forceQList.at(0)->text().toDouble() != 0){
            if(forceQList.at(0)->text().toDouble() > 0){
                QGraphicsPixmapItem * forceQ = graphicScene->addPixmap(QPixmap(":/resources/images/longPlusForce.png"));
                forceQ->pixmap() = forceQ->pixmap().scaled(forceQ->pixmap().width()*scale,forceQ->pixmap().height()*scale);
                forceQ->setParentItem(rectItem);
                forceQ->setPos(rectItem->rect().center().rx() - forceQ->pixmap().width()/2,
                               rectItem->rect().center().ry() - forceQ->pixmap().height()/2);
            } else if(forceQList.at(0)->text().toDouble() < 0){
                QGraphicsPixmapItem * forceQ = graphicScene->addPixmap(QPixmap(":/resources/images/longMinusForce.png"));
                forceQ->pixmap() = forceQ->pixmap().scaled(forceQ->pixmap().width()*scale,forceQ->pixmap().height()*scale);
                forceQ->setParentItem(rectItem);
                forceQ->setPos(rectItem->rect().center().rx() - forceQ->pixmap().width()/2,
                               rectItem->rect().center().ry() - forceQ->pixmap().height()/2);
            }
        }
    // отрисовка нагрузок на 1 и 2 узлах
    if(forceFList.at(0) != nullptr){
        if(forceFList.at(0)->text().toDouble() != 0){
            if(forceFList.at(0)->text().toDouble() > 0){
                QGraphicsPixmapItem * forceF = graphicScene->addPixmap(QPixmap(":/resources/images/vertexPlusForce.png"));
                forceF->pixmap() = forceF->pixmap().scaled(forceF->pixmap().width()*scale,forceF->pixmap().height()*scale);
                forceF->setParentItem(rectItem);
                forceF->setPos(rectItem->rect().topLeft().rx(),
                               rectItem->rect().center().ry() - forceF->pixmap().height()/2);
                forcesF.append(forceF);
            } else if(forceFList.at(0)->text().toDouble() < 0){
                QGraphicsPixmapItem * forceF = graphicScene->addPixmap(QPixmap(":/resources/images/vertexMinusForce.png"));
                forceF->pixmap() = forceF->pixmap().scaled(forceF->pixmap().width()*scale,forceF->pixmap().height()*scale);
                forceF->setParentItem(rectItem);
                forceF->setPos(rectItem->rect().topLeft().rx(),
                               rectItem->rect().center().ry() - forceF->pixmap().height()/2);
                forcesF.append(forceF);
            }
        }
    }
    if(forceFList.at(1) != nullptr){
        if(forceFList.at(1)->text().toDouble() != 0){
            if(forceFList.at(1)->text().toDouble() > 0){
                QGraphicsPixmapItem * forceF = graphicScene->addPixmap(QPixmap(":/resources/images/vertexPlusForce.png"));
                forceF->pixmap() = forceF->pixmap().scaled(forceF->pixmap().width()*scale,forceF->pixmap().height()*scale);
                forceF->setParentItem(rectItem);
                forceF->setPos(rectItem->rect().topRight().rx(),
                               rectItem->rect().center().ry() - forceF->pixmap().height()/2);
                forcesF.append(forceF);
            } else if(forceFList.at(1)->text().toDouble() < 0){
                QGraphicsPixmapItem * forceF = graphicScene->addPixmap(QPixmap(":/resources/images/vertexMinusForce.png"));
                forceF->pixmap() = forceF->pixmap().scaled(forceF->pixmap().width()*scale,forceF->pixmap().height()*scale);
                forceF->setParentItem(rectItem);
                forceF->setPos(rectItem->rect().topRight().rx(),
                               rectItem->rect().center().ry() - forceF->pixmap().height()/2);
                forcesF.append(forceF);
            }
        }
    }
    // отрисовка оси
    lineItem = graphicScene->addLine(-1000, rectItem->rect().center().ry(), 2000, rectItem->rect().center().ry(),QPen(Qt::DashDotLine));
    lineItem->setFlags(QGraphicsItem::ItemStacksBehindParent);

    leftSupport->setPos((0 - leftSupport->pixmap().width()),
                        rectItem->rect().center().ry()-leftSupport->pixmap().height()/2);
    // отрисовка остальных стержней
    if(barsList.begin()+1 != barsList.end())
        for(int i = 1; i < barsList.size(); i++){
            double x = rects.at(rects.size()-1)->rect().topRight().rx();
            double dy = (barsList.at(i).at(1)->text().toDouble() - barsList.at(i-1).at(1)->text().toDouble())*HEIGHT*scale/2;
            double y = rects.at(rects.size()-1)->rect().topRight().ry() - dy;

            QGraphicsRectItem* rectItem = graphicScene->addRect(x,y,barsList.at(i).at(0)->text().toDouble()*WIDTH*scale,
                                                                barsList.at(i).at(1)->text().toDouble()*HEIGHT*scale, QPen(Qt::black,3));
            rects.append(rectItem);

            if(forceQList.at(i) != nullptr)
                if(forceQList.at(i)->text().toDouble() != 0){
                    if(forceQList.at(i)->text().toDouble() > 0){
                        QGraphicsPixmapItem * forceQ = graphicScene->addPixmap(QPixmap(":/resources/images/longPlusForce.png"));
                        forceQ->pixmap() = forceQ->pixmap().scaled(forceQ->pixmap().width()*scale,forceQ->pixmap().height()*scale);
                        forceQ->setParentItem(rectItem);
                        forceQ->setPos(rectItem->rect().center().rx() - forceQ->pixmap().width()/2,
                                       rectItem->rect().center().ry() - forceQ->pixmap().height()/2);
                    } else if(forceQList.at(i)->text().toDouble() < 0){
                        QGraphicsPixmapItem * forceQ = graphicScene->addPixmap(QPixmap(":/resources/images/longMinusForce.png"));
                        forceQ->pixmap() = forceQ->pixmap().scaled(forceQ->pixmap().width()*scale,forceQ->pixmap().height()*scale);
                        forceQ->setParentItem(rectItem);
                        forceQ->setPos(rectItem->rect().center().rx() - forceQ->pixmap().width()/2,
                                       rectItem->rect().center().ry() - forceQ->pixmap().height()/2);
                    }
                }
            if(forceFList.at(i+1) != nullptr){
                if(forceFList.at(i+1)->text().toDouble() != 0){
                    if(forceFList.at(i+1)->text().toDouble() > 0){
                        QGraphicsPixmapItem * forceF = graphicScene->addPixmap(QPixmap(":/resources/images/vertexPlusForce.png"));
                        forceF->pixmap() = forceF->pixmap().scaled(forceF->pixmap().width()*scale,forceF->pixmap().height()*scale);
                        forceF->setParentItem(rectItem);
                        forceF->setPos(rectItem->rect().topRight().rx()/* - forceF->pixmap().width()*/
                                       ,rectItem->rect().center().ry() - forceF->pixmap().height()/2);
                        forcesF.append(forceF);
                    } else if(forceFList.at(i+1)->text().toDouble() < 0){
                        QGraphicsPixmapItem * forceF = graphicScene->addPixmap(QPixmap(":/resources/images/vertexMinusForce.png"));
                        forceF->pixmap() = forceF->pixmap().scaled(forceF->pixmap().width()*scale,forceF->pixmap().height()*scale);
                        forceF->setParentItem(rectItem);
                        forceF->setPos(rectItem->rect().topRight().rx(),
                                       rectItem->rect().center().ry() - forceF->pixmap().height()/2);
                        forcesF.append(forceF);
                    }
                }
            }
        }
    rightSupport->setPos(rects.at(rects.size()-1)->rect().topRight().rx(),
                        leftSupport->y());

    if(ui->sealingLeftCheckBox->isChecked())
        leftSupport->show();
    if(ui->sealingRightCheckBox->isChecked())
        rightSupport->show();

    ui->graphicsView->centerOn(rects.at(rects.size()/2));
}

void MainWindow::scaleIncreasePressed(){
    if((scale + 0.1) <= 1.9)
        scale += 0.1;
    if(isBarTableValid() && isForceTableValid(ui->forceFTableWidget) && isForceTableValid(ui->forceQTableWidget))
        draw();
}

void MainWindow::scaleDecreasePressed(){
    if((scale - 0.1) >= 0.1)
        scale -= 0.1;
    if(isBarTableValid() && isForceTableValid(ui->forceFTableWidget) && isForceTableValid(ui->forceQTableWidget))
        draw();
}

void MainWindow::scaleResetPressed(){
    scale = 1;
    if(isBarTableValid() && isForceTableValid(ui->forceFTableWidget) && isForceTableValid(ui->forceQTableWidget))
        draw();
}

void MainWindow::leftSupportValueChanged(const int& state){
    if(state != 2){
        ui->forceFTableWidget->item(0,0)->setFlags(Qt::ItemIsEnabled);
        ui->forceFTableWidget->item(0,0)->setFlags(ui->forceFTableWidget->item(0,0)->flags() ^ Qt::ItemIsSelectable);
        ui->forceFTableWidget->item(0,0)->setFlags(ui->forceFTableWidget->item(0,0)->flags() ^ Qt::ItemIsEditable);
        if(!isBarTableValid())
            return;
        leftSupport->hide();
    }
    else{
        qInfo() << ui->forceFTableWidget->item(0,0)->flags();
        ui->forceFTableWidget->item(0,0)->setFlags(Qt::NoItemFlags);
        ui->forceFTableWidget->item(0,0)->setText("0");
        if(!isBarTableValid())
            return;
        leftSupport->show();
    }
}

void MainWindow::rightSupportValueChanged(const int& state){
    if(barsAmount == 0)
        return;
    if(state != 2){
        ui->forceFTableWidget->item(ui->forceFTableWidget->rowCount()-1,0)->setFlags(Qt::ItemIsEnabled);
        ui->forceFTableWidget->item(ui->forceFTableWidget->rowCount()-1,0)->
                setFlags(ui->forceFTableWidget->item(ui->forceFTableWidget->rowCount()-1,0)->flags() ^ Qt::ItemIsSelectable);
        ui->forceFTableWidget->item(ui->forceFTableWidget->rowCount()-1,0)->
                setFlags(ui->forceFTableWidget->item(ui->forceFTableWidget->rowCount()-1,0)->flags() ^ Qt::ItemIsEditable);
        if(!isBarTableValid())
            return;
        rightSupport->hide();
    }
    else {
        ui->forceFTableWidget->item(ui->forceFTableWidget->rowCount()-1,0)->setFlags(Qt::NoItemFlags);
        ui->forceFTableWidget->item(ui->forceFTableWidget->rowCount()-1,0)->setText("0");
        if(!isBarTableValid())
            return;
        rightSupport->show();
    }
}

void MainWindow::about(){
    QMessageBox::about(this,tr("О программе"),
                       tr("Данная программа является реализацией системы автоматических прочностных расчетов (САПР) стержневых конструкций.\n"
                        "Автор: студент Коваленко В.М. группы ИДБ-19-10.\n"
                          "МГТУ \"СТАНКИН\" 2021г"));
}

void MainWindow::clearDataTables(){
    ui->barTableWidget->blockSignals(true);
    ui->forceFTableWidget->blockSignals(true);
    ui->forceQTableWidget->blockSignals(true);

    for(int i = 0; i < ui->barTableWidget->rowCount();i++){
        for(int j = 0; j < ui->barTableWidget->columnCount();j++){
            ui->barTableWidget->item(i,j)->setText("1");
            ui->barTableWidget->item(i,j)->setBackground(Qt::white);
        }
    }

    for(int i = 0; i < ui->forceFTableWidget->rowCount();i++){
        for(int j = 0; j < ui->forceFTableWidget->columnCount();j++){
            ui->forceFTableWidget->item(i,j)->setText("0");
            ui->forceFTableWidget->item(i,j)->setBackground(Qt::white);
        }
    }

    for(int i = 0; i < ui->forceQTableWidget->rowCount();i++){
        for(int j = 0; j < ui->forceQTableWidget->columnCount();j++){
            ui->forceQTableWidget->item(i,j)->setText("0");
            ui->forceQTableWidget->item(i,j)->setBackground(Qt::white);
        }
    }

    ui->barTableWidget->blockSignals(false);
    ui->forceFTableWidget->blockSignals(false);
    ui->forceQTableWidget->blockSignals(false);
}

void MainWindow::saveAs(){
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить как");
    QFile file(fileName);

    if(!file.open(QFile::WriteOnly|QFile::Text)){
        QMessageBox::warning(this, "Ошибка!","Невозможно открыть файл: " + file.errorString());
        return;
    }
    currentFile = fileName;
    QTextStream out(&file);
    QString text;
    out << "========= Стержни =========\n";
    out << "Количество стержней: " << QString::number(barsAmount) << " \n";
    out << "=== L === A === σ === E ===\n";
    // добавление информации о стержнях
   for(int i = 0; i < barsAmount; i++){
       text += "=" + QString::number(i+1);
       for(int j = 0; j < 4; j++){
            text += ' ' + ui->barTableWidget->item(i,j)->text();
       }
       text += '\n';
   }
   //добавление информации о нагрузках
   text += "========= Нагрузки =========\n";
   text += "======== F ======= q =======\n";
   for(int i = 0; i < barsAmount+1; i++){
       text += "=" + QString::number(i+1);
       text += ' ' + ui->forceFTableWidget->item(i,0)->text();
       if(i < barsAmount){
           text += ' ' + ui->forceQTableWidget->item(i,0)->text();
       }
       text += '\n';
   }
   text += "========= Заделки =========\n";
   if(ui->sealingLeftCheckBox->isChecked())
       text += "1 ";
   else text += "0 ";
   if(ui->sealingRightCheckBox->isChecked())
       text += "1\n";
   else text += "0\n";
   out << text;
   file.close();
}

void MainWindow::open(){
    QString fileName = QFileDialog::getOpenFileName(this,"Open file");
    QFile file(fileName);
    currentFile = fileName;

    if(!file.open(QIODevice::ReadOnly|QFile::Text)){
        QMessageBox::warning(this,"Warning","Cannot open file: " + file.errorString());
        return;
    }

    bool isBarDataFound = false;
    bool isForceDataFound = false;
    bool isBarAmountFound = false;
    bool leftSupportFound = false;
    bool rightSupportFound = false;
    int tempBarAmount = 0;
    QVector<QStringList> tempBarData;
    QVector<QStringList> tempForceData;

    QTextStream in(&file);
    while(!in.atEnd()){
        QString line = in.readLine();
        if(line.contains("Количество стержней")){
            isBarAmountFound = true;
            QStringList lineContent = line.split(' ',Qt::SkipEmptyParts);
            tempBarAmount = lineContent.at(2).toInt();
        }
        if(line.contains('L')){
            isBarDataFound = true;
            for(int i = 0;i < tempBarAmount; i++){
                line = in.readLine();
                QStringList lineContent = line.split(' ', Qt::SkipEmptyParts);
                tempBarData.append(lineContent);
            }
        }
        if(line.contains('F')){
            isForceDataFound = true;
            for(int i = 0;i < tempBarAmount+1; i++){
                line = in.readLine();
                QStringList lineContent = line.split(' ', Qt::SkipEmptyParts);
                tempForceData.append(lineContent);
            }
        }
        if(line.contains("Заделки")){
            line = in.readLine();
            QStringList lineContent = line.split(' ', Qt::SkipEmptyParts);
            if(lineContent.size() == 2){
                if(lineContent[0] == '1')
                    leftSupportFound = true;
                if(lineContent[1] == '1')
                    rightSupportFound = true;
            }
            else{
                QMessageBox::warning(this,"Warning!","Некорректный файл!");
                file.close();
                return;
            }
        }
    }
    clearDataTables();

    if(!isBarAmountFound || !isForceDataFound || !isBarDataFound){
        QMessageBox::warning(this,"Warning!","Некорректный файл!");
        file.close();
        return;
    }
    else{
        //все ли данные присутствую в файле вообще
        if(tempBarData.size() != tempBarAmount || tempForceData.size() != tempBarAmount+1){
            QMessageBox::warning(this,"Warning!","Некорректный файл!");
            file.close();
            return;
        }
        // все ли данные о стержне присутствуют
        for(int i = 0; i < tempBarAmount;i++){
            if(tempBarData[i].size() != 5){
                QMessageBox::warning(this,"Warning!","Некорректный файл!");
                file.close();
                return;
            }
        }
        // все ли данные о нагрузках присутствуют
        for(int i=0;i<tempBarAmount+1;i++)
            if(i != tempBarAmount){
                if(tempForceData[i].size()!=3){
                    QMessageBox::warning(this,"Warning!","Некорректный файл!");
                    file.close();
                    return;
                }
            }
            else
                if(tempForceData[i].size()!=2){
                    QMessageBox::warning(this,"Warning!","Некорректный файл!");
                    file.close();
                    return;
                }
        // заполнение таблиц
        ui->barAmountSpinBox->setValue(tempBarAmount);
        for(int i = 0; i < barsAmount; i++){
            for(int j = 0; j < ui->barTableWidget->columnCount(); j++){
                ui->barTableWidget->item(i,j)->setText(tempBarData[i][j+1]);
            }
        }
        for(int i = 0;i < barsAmount+1; i++){
            if(i != barsAmount){
                ui->forceFTableWidget->item(i,0)->setText(tempForceData[i][1]);
                ui->forceQTableWidget->item(i,0)->setText(tempForceData[i][2]);
            }
            else
                ui->forceFTableWidget->item(i,0)->setText(tempForceData[i][1]);
        }
        ui->sealingLeftCheckBox->setChecked(leftSupportFound);
        ui->sealingRightCheckBox->setChecked(rightSupportFound);
    }

    file.close();
}

void MainWindow::exit(){
    QCoreApplication::exit();
}

