#include "headers/mainwindow.h"
#include "ui_mainwindow.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>

const double HEIGHT = 45;
const double WIDTH = 60;
const double SPACE_AMOUNT = 30;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    currentFile = "PLACEHOLDER_TEXT";

    QAction *actionAbout = ui->menubar->addAction("About");
    ui->tabWidget->setTabVisible(1,false);
    ui->tabWidget->setTabVisible(2,false);
    ui->actionTableView->setCheckable(true);
    ui->menuPostprocessorParameters->setEnabled(false);
    ui->actionSaveResult->setEnabled(false);

    graphicScene = new QGraphicsScene;
    postprocessorGraphicScene = new QGraphicsScene;
    ui->graphicsView->setScene(graphicScene);
    ui->epuresGraphicsView->setScene(postprocessorGraphicScene);

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
    connect(ui->calcPushButton, &QPushButton::pressed, this, &MainWindow::calculate);
    connect(ui->actionTableView, &QAction::triggered, this, &MainWindow::ppShowTableView);
    connect(ui->actionNewFile, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::save);
    connect(ui->actionSaveResult, &QAction::triggered, this, &MainWindow::saveResult);
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
            forceQList.replace(i,item);
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
    if(!isBarTableValid() || !isForceTableValid(ui->forceFTableWidget) || !isForceTableValid(ui->forceQTableWidget) ||
            (ui->sealingLeftCheckBox->isChecked() == false && ui->sealingRightCheckBox->isChecked() == false)){
        QMessageBox::warning(this,"Warning!","Некорректные данные!");
        return;
    }

    // инициализация массивов данных
    QList<QList<double>> AList;
    QList<double> bList;

    AList.resize(barsAmount+1);
    for(int i = 0; i < AList.size(); i++){
        AList[i].resize(barsAmount+1);
        for(int j = 0; j < AList[i].size();j++)
            AList[i][j] = 0;
    }

    bList.resize(barsAmount+1);
    for(int i = 0; i < bList.size();i++)
        bList[i] = 0;

    // заполнение матрицы A
    for(int i = 0; i < barsAmount + 1; i++){
        if(i == 0){
            AList[i][i] = (barsList[i][3]->text().toDouble() * barsList[i][1]->text().toDouble())/barsList[i][0]->text().toDouble();
            AList[i][i+1] = -1 * AList[i][i];
            AList[i+1][i] = -1 * AList[i][i];
        }
        else if(i == barsAmount){
            AList[i][i] = (barsList[i-1][3]->text().toDouble() * barsList[i-1][1]->text().toDouble())/barsList[i-1][0]->text().toDouble();
            AList[i][i-1] = -1 * (barsList[i-1][3]->text().toDouble() * barsList[i-1][1]->text().toDouble())/barsList[i-1][0]->text().toDouble();
            AList[i-1][i] = -1 * (barsList[i-1][3]->text().toDouble() * barsList[i-1][1]->text().toDouble())/barsList[i-1][0]->text().toDouble();
        }
        else{
            AList[i][i] = (barsList[i-1][3]->text().toDouble() * barsList[i-1][1]->text().toDouble())/barsList[i-1][0]->text().toDouble() +
                    (barsList[i][3]->text().toDouble() * barsList[i][1]->text().toDouble())/barsList[i][0]->text().toDouble();
            AList[i][i+1] = -1 * (barsList[i][3]->text().toDouble() * barsList[i][1]->text().toDouble())/barsList[i][0]->text().toDouble();
            AList[i+1][i] = -1 * (barsList[i][3]->text().toDouble() * barsList[i][1]->text().toDouble())/barsList[i][0]->text().toDouble();
        }
    }
    if(ui->sealingLeftCheckBox->isChecked()){
        AList[0][0] = 1;
        for(int i = 1; i < AList.size(); i++)
            AList[0][i] = 0;
    }
    if(ui->sealingRightCheckBox->isChecked()){
        AList[AList.size() - 1][AList.size() - 1] = 1;
        for(int i = 0; i < AList.size() - 1; i++)
            AList[AList.size() - 1][i] = 0;
    }

    //заполнение матрицы b
    for(int i = 0; i < barsAmount + 1; i++){
        if(i == 0){
            if(!ui->sealingLeftCheckBox->isChecked())
                bList[i] = forceFList[i]->text().toDouble() + (forceQList[i]->text().toDouble()*barsList[i][0]->text().toDouble())/2;
            else
                bList[i] = 0;
        }
        else if(i == barsAmount){
            if(!ui->sealingRightCheckBox->isChecked())
                bList[i] = forceFList[i]->text().toDouble() + (forceQList[i-1]->text().toDouble()*barsList[i-1][0]->text().toDouble())/2;
            else
                bList[i] = 0;
        }
        else{
            bList[i] = forceFList[i]->text().toDouble() + (forceQList[i-1]->text().toDouble()*barsList[i-1][0]->text().toDouble())/2 +
                    (forceQList[i]->text().toDouble()*barsList[i][0]->text().toDouble())/2;
        }
    }

    resultAList = AList;
    resultBList = bList;
    resultDeltaList = Gauss(AList, bList);

    if(!resultDeltaList.isEmpty()){
        // подсчет Nx
        resultNXList.clear();
        for(int i = 0; i < barsAmount; i++){
            QList<double> NXPerSpaceList;
            for(int j = 0; j < SPACE_AMOUNT*barsList[i][0]->text().toDouble();j++){
                double NX = barsList[i][3]->text().toDouble()*barsList[i][1]->text().toDouble()/barsList[i][0]->text().toDouble()
                        * (resultDeltaList[i+1] - resultDeltaList[i]) + forceQList[i]->text().toDouble()*barsList[i][0]->text().toDouble()/
                        2*(1 - 2*(j * barsList[i][0]->text().toDouble()/(SPACE_AMOUNT*std::pow(barsList[i][0]->text().toDouble(),2))));
                NXPerSpaceList.push_back(NX);
            }
            resultNXList.push_back(NXPerSpaceList);
        }

        // подсчет Ux
        resultUXList.clear();
        for(int i = 0; i < barsAmount; i++){
            QList<double> UXPerSpaceList;
            for(int j = 0; j < SPACE_AMOUNT*barsList[i][0]->text().toDouble();j++){
                double UX = resultDeltaList[i] +
                        ((j * barsList[i][0]->text().toDouble()/(SPACE_AMOUNT*barsList[i][0]->text().toDouble()))/barsList[i][0]->text().toDouble()) *
                        (resultDeltaList[i+1] - resultDeltaList[i]) + (forceQList[i]->text().toDouble() * std::pow(barsList[i][0]->text().toDouble(),2))/
                        (2*barsList[i][3]->text().toDouble()*barsList[i][1]->text().toDouble())*(j * barsList[i][0]->text().toDouble()/
                        (SPACE_AMOUNT*barsList[i][0]->text().toDouble()))/barsList[i][0]->text().toDouble()*
                        (1 - (j * barsList[i][0]->text().toDouble()/(SPACE_AMOUNT*barsList[i][0]->text().toDouble()))/barsList[i][0]->text().toDouble());
                UXPerSpaceList.push_back(UX);
            }
            resultUXList.push_back(UXPerSpaceList);
        }
        // подсчет σ_x
        resultSigmaXList.clear();
        for(int i = 0; i < barsAmount; i++){
            QList<double> SigmaXList;
            for(int j = 0; j < SPACE_AMOUNT*barsList[i][0]->text().toDouble(); j++){
                double SigmaX = resultNXList[i][j]/barsList[i][1]->text().toDouble();
                SigmaXList.push_back(SigmaX);
            }
            resultSigmaXList.push_back(SigmaXList);
        }
        // конец расчетов в процессоре
        ui->tabWidget->setTabVisible(1,true);
        ui->menuPostprocessorParameters->setEnabled(true);
        ui->actionSaveResult->setEnabled(true);
        configurePostprocessor();
    }
}
// настройка постпроцессора
void MainWindow::configurePostprocessor(){
    ui->bResultTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->aResultTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->deltaResultTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // заполнение таблицы матрицы A
    ui->aResultTableWidget->setRowCount(resultAList.size());
    ui->aResultTableWidget->setColumnCount(resultAList[0].size());
    for(int i = 0; i < resultAList.size();i++){
        for(int j = 0; j < resultAList[i].size();j++){
            QTableWidgetItem *item = new QTableWidgetItem(QString::number(resultAList[i][j]));
            item->setBackground(Qt::white);
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(Qt::NoItemFlags);
            item->setFlags(Qt::ItemIsEnabled);
            ui->aResultTableWidget->setItem(i,j,item);
        }
    }

    // заполнение таблицы матрицы B
    ui->bResultTableWidget->setRowCount(resultBList.size());
    ui->bResultTableWidget->setColumnCount(1);
    for(int i = 0; i < resultBList.size();i++){
        QTableWidgetItem *item = new QTableWidgetItem(QString::number(resultBList[i]));
        item->setBackground(Qt::white);
        item->setTextAlignment(Qt::AlignCenter);
        item->setFlags(Qt::NoItemFlags);
        item->setFlags(Qt::ItemIsEnabled);
        ui->bResultTableWidget->setItem(i,0,item);
    }

    // заполнение таблицы матрицы delta
    ui->deltaResultTableWidget->setRowCount(resultDeltaList.size());
    ui->deltaResultTableWidget->setColumnCount(1);
    for(int i = 0; i < resultDeltaList.size();i++){
        QTableWidgetItem *item = new QTableWidgetItem(QString::number(resultDeltaList[i]));
        item->setBackground(Qt::white);
        item->setTextAlignment(Qt::AlignCenter);
        item->setFlags(Qt::NoItemFlags);
        item->setFlags(Qt::ItemIsEnabled);
        ui->deltaResultTableWidget->setItem(i,0,item);
    }

    // заполнение таблицы Nx
    short maxSize = 0;
    for(int i = 0; i < resultNXList.size();i++){
        if(resultNXList[i].size() > maxSize)
            maxSize = resultNXList[i].size();
    }
    ui->resultNxTableWidget->setRowCount(maxSize);
    ui->resultNxTableWidget->setColumnCount(barsAmount*2);
    ui->resultNxTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->resultNxTableWidget->verticalHeader()->hide();
    int currentBar = 0;
    for(int i = 0; i < ui->resultNxTableWidget->columnCount(); i += 2){
        QTableWidgetItem* itemLHeader = new QTableWidgetItem("L");
        QTableWidgetItem* itemHeader = new QTableWidgetItem(QString::number(currentBar+1));
        ui->resultNxTableWidget->setHorizontalHeaderItem(i, itemLHeader);
        ui->resultNxTableWidget->setHorizontalHeaderItem(i+1, itemHeader);
        for(int j = 0; j < resultNXList[currentBar].size();j++){
            QTableWidgetItem* item = new QTableWidgetItem(QString::number(resultNXList[currentBar][j]));
            QTableWidgetItem* itemL = new QTableWidgetItem(QString::number(((double)j+1)/resultNXList[currentBar].size()) + "L");
            item->setBackground(Qt::white);
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(Qt::NoItemFlags);
            item->setFlags(Qt::ItemIsEnabled);
            itemL->setFlags(Qt::NoItemFlags);
            itemL->setFlags(Qt::ItemIsEnabled);
            itemL->setTextAlignment(Qt::AlignCenter);
            ui->resultNxTableWidget->setItem(j,i+1,item);
            ui->resultNxTableWidget->setItem(j,i,itemL);
        }
        currentBar++;
    }
    currentBar = 0;
    // заполнение таблицы Ux
    ui->resultUxTableWidget->setRowCount(maxSize);
    ui->resultUxTableWidget->setColumnCount(barsAmount*2);
    ui->resultUxTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->resultUxTableWidget->verticalHeader()->hide();
    for(int i = 0; i < ui->resultUxTableWidget->columnCount(); i += 2){
        QTableWidgetItem* itemLHeader = new QTableWidgetItem("L");
        QTableWidgetItem* itemHeader = new QTableWidgetItem(QString::number(currentBar+1));
        ui->resultUxTableWidget->setHorizontalHeaderItem(i, itemLHeader);
        ui->resultUxTableWidget->setHorizontalHeaderItem(i+1, itemHeader);
        for(int j = 0; j < resultUXList[currentBar].size(); j++){
            QTableWidgetItem* item = new QTableWidgetItem(QString::number(resultUXList[currentBar][j]));
            QTableWidgetItem* itemL = new QTableWidgetItem(QString::number(((double)j+1)/resultUXList[currentBar].size()) + "L");
            item->setBackground(Qt::white);
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(Qt::NoItemFlags);
            item->setFlags(Qt::ItemIsEnabled);
            itemL->setFlags(Qt::NoItemFlags);
            itemL->setFlags(Qt::ItemIsEnabled);
            itemL->setTextAlignment(Qt::AlignCenter);
            ui->resultUxTableWidget->setItem(j,i+1,item);
            ui->resultUxTableWidget->setItem(j,i,itemL);
        }
        currentBar++;
    }
    currentBar = 0;
    // заполнение таблицы SigmaX
    ui->resultSigmaTableWidget->setRowCount(maxSize);
    ui->resultSigmaTableWidget->setColumnCount(barsAmount*2);
    ui->resultSigmaTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->resultSigmaTableWidget->verticalHeader()->hide();
    for(int i = 0; i < ui->resultSigmaTableWidget->columnCount(); i += 2){
        QTableWidgetItem* itemLHeader = new QTableWidgetItem("L");
        QTableWidgetItem* itemHeader = new QTableWidgetItem(QString::number(currentBar+1));
        ui->resultSigmaTableWidget->setHorizontalHeaderItem(i, itemLHeader);
        ui->resultSigmaTableWidget->setHorizontalHeaderItem(i+1, itemHeader);
        for(int j = 0; j < resultSigmaXList[currentBar].size();j++){
            QTableWidgetItem* item = new QTableWidgetItem(QString::number(resultSigmaXList[currentBar][j]));
            QTableWidgetItem* itemL = new QTableWidgetItem(QString::number(((double)j+1)/resultSigmaXList[currentBar].size()) + "L");
            if(abs(resultSigmaXList[currentBar][j]) > barsList[currentBar][2]->text().toDouble())
                item->setBackground(Qt::red);
            else item->setBackground(Qt::white);
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(Qt::NoItemFlags);
            item->setFlags(Qt::ItemIsEnabled);
            itemL->setFlags(Qt::NoItemFlags);
            itemL->setFlags(Qt::ItemIsEnabled);
            itemL->setTextAlignment(Qt::AlignCenter);
            ui->resultSigmaTableWidget->setItem(j,i+1,item);
            ui->resultSigmaTableWidget->setItem(j,i,itemL);
        }
        currentBar++;
    }

    ui->NXCheckBox->setChecked(true);
    ui->UXCheckBox->setChecked(false);
    ui->SigmaCheckBox->setChecked(false);
    drawPostprocessor();
    ui->tabWidget->setCurrentIndex(1);
}
// не совсем оригинальная разработка :)
QList<double> MainWindow::Gauss(QList<QList<double>> &matrixA, QList<double> &matrixB){
    QList<double> result;
    int k = 0;
    int index;
    double maxValue;
    result.resize(matrixA.size());

    for(int i = 0; i < result.size(); i++)
        result[i] = 0.0;

    // прямой ход
    while(k < matrixA.size()){
        maxValue = abs(matrixA[k][k]);
        index = k;

        for(int i = k + 1; i < matrixA.size();i++)
            if(abs(matrixA[i][k]) > maxValue){
                maxValue = abs(matrixA[i][k]);
                index = i;
            }
        if(maxValue < 0.00001){
            QMessageBox::warning(this,"Ошибка!","Невозможно получить решение из-за нулевого столбца " + QString::number(index) + " матрицы A");
            result.clear();
            return result;
        }
        // смена X[i]
        for(int j = 0; j < matrixA.size(); j++){
            double temp = matrixA[k][j];
            matrixA[k][j] = matrixA[index][j];
            matrixA[index][j] = temp;
        }
        // смена Y
        double temp = matrixB[k];
        matrixB[k] = matrixB[index];
        matrixB[index] = temp;

        for(int i = k; i < matrixA.size(); i++){
            double temp = matrixA[i][k];
            if(abs(temp) < 0.00001) continue;
            for(int j = 0; j < matrixA.size(); j++)
                matrixA[i][j] /= temp;
            matrixB[i] /= temp;
            if(i == k) continue;
            for(int j = 0; j < matrixA.size();j++)
                matrixA[i][j] -= matrixA[k][j];
            matrixB[i] -= matrixB[k];
        }
        k++;
    }

    // обратный ход
    for(k = matrixA.size()-1; k >= 0; k--){
        result[k] = matrixB[k];
        for(int i = 0; i < k; i++)
            matrixB[i] -= matrixA[i][k]*result[k];
    }

    return result;
}
// P.S. Отрисовка происходит с учетом того факта, что все данные о стержнях корректны
void MainWindow::draw(){
    graphicScene->clear();

    QList<QGraphicsRectItem*> rects;
    QList<QGraphicsPixmapItem*> forcesF;

    leftSupport = graphicScene->addPixmap(QPixmap(":/resources/images/leftSupport.png"));
    rightSupport = graphicScene->addPixmap(QPixmap(":/resources/images/rightSupport.png"));
    leftSupport->hide();
    rightSupport->hide();

    // отрисовка 1-го стержня
    QGraphicsRectItem* rectItem = graphicScene->addRect(0,0,barsList.at(0).at(0)->text().toDouble()*WIDTH,
                                                        barsList.at(0).at(1)->text().toDouble()*HEIGHT, QPen(Qt::black,3));
    rects.append(rectItem);
    if(forceQList.at(0) != nullptr)
        if(forceQList.at(0)->text().toDouble() != 0){
            if(forceQList.at(0)->text().toDouble() > 0){
                QGraphicsPixmapItem * forceQ = graphicScene->addPixmap(QPixmap(":/resources/images/longPlusForce.png"));
                forceQ->setParentItem(rectItem);
                forceQ->setPos(rectItem->rect().center().rx() - forceQ->pixmap().width()/2,
                               rectItem->rect().center().ry() - forceQ->pixmap().height()/2);
            } else if(forceQList.at(0)->text().toDouble() < 0){
                QGraphicsPixmapItem * forceQ = graphicScene->addPixmap(QPixmap(":/resources/images/longMinusForce.png"));
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
                forceF->setParentItem(rectItem);
                forceF->setPos(rectItem->rect().topLeft().rx(),
                               rectItem->rect().center().ry() - forceF->pixmap().height()/2);
                forcesF.append(forceF);
            } else if(forceFList.at(0)->text().toDouble() < 0){
                QGraphicsPixmapItem * forceF = graphicScene->addPixmap(QPixmap(":/resources/images/vertexMinusForce.png"));
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
                forceF->setParentItem(rectItem);
                forceF->setPos(rectItem->rect().topRight().rx(),
                               rectItem->rect().center().ry() - forceF->pixmap().height()/2);
                forcesF.append(forceF);
            } else if(forceFList.at(1)->text().toDouble() < 0){
                QGraphicsPixmapItem * forceF = graphicScene->addPixmap(QPixmap(":/resources/images/vertexMinusForce.png"));
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
            double dy = (barsList.at(i).at(1)->text().toDouble() - barsList.at(i-1).at(1)->text().toDouble())*HEIGHT/2;
            double y = rects.at(rects.size()-1)->rect().topRight().ry() - dy;

            QGraphicsRectItem* rectItem = graphicScene->addRect(x,y,barsList.at(i).at(0)->text().toDouble()*WIDTH,
                                                                barsList.at(i).at(1)->text().toDouble()*HEIGHT, QPen(Qt::black,3));
            rects.append(rectItem);

            if(forceQList.at(i) != nullptr)
                if(forceQList.at(i)->text().toDouble() != 0){
                    if(forceQList.at(i)->text().toDouble() > 0){
                        QGraphicsPixmapItem * forceQ = graphicScene->addPixmap(QPixmap(":/resources/images/longPlusForce.png"));
                        forceQ->setParentItem(rectItem);
                        forceQ->setPos(rectItem->rect().center().rx() - forceQ->pixmap().width()/2,
                                       rectItem->rect().center().ry() - forceQ->pixmap().height()/2);
                    } else if(forceQList.at(i)->text().toDouble() < 0){
                        QGraphicsPixmapItem * forceQ = graphicScene->addPixmap(QPixmap(":/resources/images/longMinusForce.png"));
                        forceQ->setParentItem(rectItem);
                        forceQ->setPos(rectItem->rect().center().rx() - forceQ->pixmap().width()/2,
                                       rectItem->rect().center().ry() - forceQ->pixmap().height()/2);
                    }
                }
            if(forceFList.at(i+1) != nullptr){
                if(forceFList.at(i+1)->text().toDouble() != 0){
                    if(forceFList.at(i+1)->text().toDouble() > 0){
                        QGraphicsPixmapItem * forceF = graphicScene->addPixmap(QPixmap(":/resources/images/vertexPlusForce.png"));
                        forceF->setParentItem(rectItem);
                        forceF->setPos(rectItem->rect().topRight().rx()/* - forceF->pixmap().width()*/
                                       ,rectItem->rect().center().ry() - forceF->pixmap().height()/2);
                        forcesF.append(forceF);
                    } else if(forceFList.at(i+1)->text().toDouble() < 0){
                        QGraphicsPixmapItem * forceF = graphicScene->addPixmap(QPixmap(":/resources/images/vertexMinusForce.png"));
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
void MainWindow::drawPostprocessor(){
    postprocessorGraphicScene->clear();
    NXGraphicItems.clear();
    UXGraphicItems.clear();
    SigmaGraphicItems.clear();

    double currentBar = 0;
    double position = 0;
    // отрисовка Nx
    while(currentBar != barsList.size())
    {
        for(int j = 0; j < resultNXList[currentBar].size(); j++){
            QGraphicsLineItem* lineItem = postprocessorGraphicScene->addLine(position,0,position,0 - resultNXList[currentBar][j]*15,QPen(Qt::blue,2));
            position += 3;
            NXGraphicItems.push_back(lineItem);
        }
        currentBar++;
    }
    // отрисовка оси
    QGraphicsLineItem* lineItem = postprocessorGraphicScene->addLine(NXGraphicItems[0]->x()-10, 0, position+10, 0, QPen(Qt::DashDotLine));
    lineItem->setFlags(QGraphicsItem::ItemStacksBehindParent);
    // отрисовка Ux
    position = 0;
    currentBar = 0;
    while(currentBar != barsList.size())
    {
        for(int j = 0; j < resultUXList[currentBar].size(); j++){
            QGraphicsLineItem* lineItem = postprocessorGraphicScene->addLine(position,0,position,0 - resultUXList[currentBar][j]*15,QPen(Qt::red,2));
            position += 3;
            UXGraphicItems.push_back(lineItem);
            lineItem->hide();
        }
        currentBar++;
    }
    // отрисовка sigmaX
    position = 0;
    currentBar = 0;
    while(currentBar != barsList.size())
    {
        for(int j = 0; j < resultSigmaXList[currentBar].size(); j++){
            QGraphicsLineItem* lineItem = postprocessorGraphicScene->addLine(position,0,position,0 - resultSigmaXList[currentBar][j]*15,QPen(Qt::green,2));
            position += 3;
            SigmaGraphicItems.push_back(lineItem);
            lineItem->hide();
        }
        currentBar++;
    }
    ui->epuresGraphicsView->scene()->clearSelection();
    ui->epuresGraphicsView->scene()->setSceneRect(ui->epuresGraphicsView->scene()->itemsBoundingRect());
    ui->epuresGraphicsView->centerOn(NXGraphicItems[NXGraphicItems.size()/2]);
}
// масштабирование рисунка. Автор: Марк Бобровских
void MainWindow::scaleView(qreal scaleFactor)
{
    qreal factor = ui->graphicsView->transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.5 || factor > 5)
        return;

    if(ui->tabWidget->currentIndex() == 1 || ui->tabWidget->currentIndex() == 2)
        return;

    ui->graphicsView->scale(scaleFactor, scaleFactor);
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    scaleView(pow(2., -event->angleDelta().y() / 240.0));
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

void MainWindow::save(){
    QString fileName = "Construction.txt";
    if(currentFile != "PLACEHOLDER_TEXT"){
        fileName = currentFile;
    }
    QFile file(fileName);
    if(!file.open(QFile::WriteOnly|QFile::Text)){
        QMessageBox::warning(this, "Ошибка!","Невозможно открыть файл: " + file.errorString());
        return;
    }

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

void MainWindow::saveResult(){
    QString fileName = "Construction_result.txt";
    if(currentFile != "PLACEHOLDER_TEXT"){
        int dotPos = currentFile.indexOf(".");
        fileName = currentFile.insert(dotPos,"_result");
    }
    QFile file(fileName);
    if(!file.open(QFile::WriteOnly|QFile::Text)){
        QMessageBox::warning(this, "Ошибка!","Невозможно открыть файл: " + file.errorString());
        return;
    }
    QTextStream out(&file);
    QString text;

    text += "=== Итоговая матрица A ===\n";
    for(int i = 0; i < resultAList.size();i++){
        for(int j = 0; j < resultAList[i].size();j++){
            text += ' ' + QString::number(resultAList[i][j]);
        }
        text += '\n';
    }
    text += "=== Итоговая матрица b ===\n";
    for(int i = 0; i < resultBList.size(); i++){
        text += ' ' + QString::number(resultBList[i]);
    }
    text += '\n';
    text += "=== Итоговая матрица delta ===\n";
    for(int i = 0; i < resultDeltaList.size(); i++){
        text += ' ' + QString::number(resultDeltaList[i]);
    }
    text += '\n';
    text += "=== Nx ===\n";
    for(int i = 0; i < barsAmount; i++){
        text += "Стержень " + QString::number(i+1) + ":\n";
        text += " L:= ";
        for(int j = 0; j < resultNXList[i].size();j++)
            text += QString::number(((double)j+1)/resultNXList[i].size()) + "L ";
        text += '\n';
        text += " Nx:= ";
        for(int j = 0; j < resultNXList[i].size();j++)
            text += QString::number(resultNXList[i][j]) + ' ';
        text += '\n';
    }
    text += "=== Ux ===\n";
    for(int i = 0; i < barsAmount; i++){
        text += "Стержень " + QString::number(i+1) + ":\n";
        text += " L:= ";
        for(int j = 0; j < resultUXList[i].size();j++)
            text += QString::number(((double)j+1)/resultUXList[i].size()) + "L ";
        text += '\n';
        text += " Ux:= ";
        for(int j = 0; j < resultUXList[i].size();j++)
            text += QString::number(resultUXList[i][j]) + ' ';
        text += '\n';
    }
    text += "=== SigmaX ===\n";
    for(int i = 0; i < barsAmount; i++){
        text += "Стержень " + QString::number(i+1) + ":\n";
        text += " L:= ";
        for(int j = 0; j < resultSigmaXList[i].size();j++)
            text += QString::number(((double)j+1)/resultSigmaXList[i].size()) + "L ";
        text += '\n';
        text += " SigmaX:= ";
        for(int j = 0; j < resultSigmaXList[i].size();j++)
            text += QString::number(resultSigmaXList[i][j]) + ' ';
        text += '\n';
    }
    out << text;
    file.close();
    saveEpuresAsPictures(ui->NXCheckBox->isChecked(), ui->UXCheckBox->isChecked(), ui->SigmaCheckBox->isChecked());
}

void MainWindow::saveEpuresAsPictures(bool NX, bool UX, bool Sigma){
    ui->NXCheckBox->setChecked(true);
    ui->UXCheckBox->setChecked(false);
    ui->SigmaCheckBox->setChecked(false);

    for(int i = 0; i < 3; i++){
        if(i == 1){
            ui->NXCheckBox->setChecked(false);
            ui->UXCheckBox->setChecked(true);
        }
        else if(i == 2){
            ui->UXCheckBox->setChecked(false);
            ui->SigmaCheckBox->setChecked(true);
        }

        ui->epuresGraphicsView->scene()->clearSelection();                                                      // Отчистка сцены от выделений (они входят в рендер)
        ui->epuresGraphicsView->scene()->setSceneRect(ui->epuresGraphicsView->scene()->itemsBoundingRect());    // Сжатие сцены до ее bounding contents
        QImage image(ui->epuresGraphicsView->scene()->sceneRect().size().toSize(), QImage::Format_ARGB32);      // Создание изображение по размерам сцены
        image.fill(Qt::transparent);                                                                            // заполнение изображения "прозрачными" пикселями

        QPainter painter(&image);
        ui->epuresGraphicsView->scene()->render(&painter);                                                      // рендер сцены

        switch(i){
        case 0:
            image.save("NX.png");
            break;
        case 1:
            image.save("UX.png");
            break;
        case 2:
            image.save("SigmaX.png");
            break;
        }
    }
    ui->NXCheckBox->setChecked(NX);                                                                             // возращение значений изначально выбранных эпюр
    ui->UXCheckBox->setChecked(UX);
    ui->SigmaCheckBox->setChecked(Sigma);
}

void MainWindow::hidePostProcessor(){
    ui->postprocessorTableView->hide();
    ui->postprocessor->hide();
    ui->menuPostprocessorParameters->setEnabled(false);
}

void MainWindow::resetPostProcessor(){
    resultNXList.clear();
    resultUXList.clear();
    resultSigmaXList.clear();
    resultAList.clear();
    resultBList.clear();
    resultDeltaList.clear();
    ui->epuresGraphicsView->scene()->clear();
    NXGraphicItems.clear();
    UXGraphicItems.clear();
    SigmaGraphicItems.clear();
    ui->actionTableView->setChecked(false);
}

void MainWindow::newFile(){
    QMessageBox::StandardButton userReply;
    userReply = QMessageBox::question(this, "Уведомление","Создать новую конструкцию?", QMessageBox::Yes | QMessageBox::No);
    if(userReply == QMessageBox::Yes){
        currentFile = "PLACEHOLDER_TEXT";
        clearDataTables();
        ui->sealingLeftCheckBox->setChecked(false);
        ui->sealingRightCheckBox->setChecked(false);
        resultAList.clear();
        resultBList.clear();
        resultDeltaList.clear();
        resultNXList.clear();
        resultUXList.clear();
        resultSigmaXList.clear();
        ui->epuresGraphicsView->scene()->clear();
        ui->tabWidget->setTabVisible(1, false);
        ui->tabWidget->setTabVisible(2, false);
        ui->actionSaveResult->setEnabled(false);
        ui->actionTableView->setChecked(false);
        ui->menuPostprocessorParameters->setEnabled(false);
    }
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
    ui->tabWidget->setTabVisible(1,false);
    ui->NXCheckBox->setChecked(false);
    ui->UXCheckBox->setChecked(false);
    ui->SigmaCheckBox->setChecked(false);
    resultNXList.clear();
    resultUXList.clear();
    resultSigmaXList.clear();
    file.close();
}

void MainWindow::exit(){
    QCoreApplication::exit();
}

void MainWindow::on_NXCheckBox_stateChanged(int arg1)
{
    if(ui->NXCheckBox->isChecked()){
       for(auto i: NXGraphicItems)
           i->show();
    }
    else for(auto i: NXGraphicItems)
        i->hide();
}

void MainWindow::on_UXCheckBox_stateChanged(int arg1)
{
    if(ui->UXCheckBox->isChecked()){
       for(auto i: UXGraphicItems)
           i->show();
    }
    else for(auto i: UXGraphicItems)
        i->hide();
}

void MainWindow::on_SigmaCheckBox_stateChanged(int arg1)
{
    if(ui->SigmaCheckBox->isChecked()){
       for(auto i: SigmaGraphicItems)
           i->show();
    }
    else for(auto i: SigmaGraphicItems)
        i->hide();
}

void MainWindow::ppShowTableView(){
    if(ui->actionTableView->isChecked()){
        ui->tabWidget->setTabVisible(2,true);
    }
    else ui->tabWidget->setTabVisible(2, false);
}
