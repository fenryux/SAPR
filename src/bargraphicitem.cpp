#include "headers/bargraphicitem.h"

void BarGraphicItem::setRectItem(QGraphicsRectItem* item){
    _rectItem = item;
}

void BarGraphicItem::setA(const double &value){
    _A = value;
}

void BarGraphicItem::setL(const double &value){
    _L = value;
}

QGraphicsRectItem* BarGraphicItem::item(){
    return _rectItem;
}

double BarGraphicItem::A(){
    return _A;
}

double BarGraphicItem::L(){
    return _L;
}
