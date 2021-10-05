#ifndef BARGRAPHICITEM_H
#define BARGRAPHICITEM_H

#include <QtWidgets>

class BarGraphicItem
{
public:
    BarGraphicItem(QGraphicsRectItem *rectItem, const double& L, const double& A):
        _rectItem(rectItem), _L(L), _A(A)
    {};
    BarGraphicItem(){};
    void setRectItem(QGraphicsRectItem* item);
    void setL(const double& value);
    void setA(const double& value);
    double L();
    double A();
    QGraphicsRectItem* item();
    // возможно переопределить пару операторов
private:
    QGraphicsRectItem *_rectItem;
    double _L;
    double _A;
};

#endif // BARGRAPHICITEM_H
