#ifndef MYITEM_H
#define MYITEM_H

#include <QGraphicsItem>//图元类
#include <QObject>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <QString>
#include <QPixmap>//图像类

class MyItem : public QObject,public QGraphicsItem
{
    Q_OBJECT
public:
    explicit MyItem(QString path,bool status,int dir);
    //重写方法，返回图元所在的位置
    QRectF boundingRect() const;
    //重写方法，重绘图元
    void paint(QPainter * painter,const QStyleOptionGraphicsItem * option,QWidget * widget=Q_NULLPTR);

    void advance(int phase);
private:
    QPixmap img;
    bool status;//是否移动   true移动，false固定
    int dir; // 1:向右, -1:向左
signals:
    void boom();//碰撞信号
public slots:
};

#endif // MYITEM_H
