#include "myitem.h"
#include<QtDebug>
MyItem::MyItem(QString path, bool status,int dir)
{
    this->img.load(path);
    this->status = status;
    this->dir=dir;
}

QRectF MyItem::boundingRect() const
{
    return QRectF(-this->img.width()/2, -this->img.height()/2, this->img.width(), this->img.height());
}

void MyItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->drawPixmap(-this->img.width()/2, -this->img.height()/2,
                        this->img.width(), this->img.height(), this->img);
}

void MyItem::advance(int phase)
{
    if(this->status)
    {
        if(this->collidingItems().size()>0)//碰撞检测
        {
            qDebug()<<"bool";
            this->status=false;
            emit boom();//触发碰撞信号，定时器那么发送完信号由他的上级接收，他的上级就是this就是myview
            return;//但是现在定时器还没结束
        }
        this->setPos(mapToScene(5*(this->dir),0));//maptoscene有两个参数 水平方向移动的距离 +-代表方向 +向右  -向左
    }
    //此时还不可以移动，因为定时器还没开
}
