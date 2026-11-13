#ifndef CAPTCHA_H
#define CAPTCHA_H
#include <QWidget>
#include <QString>
#include <QPainter>
#include <QMouseEvent>
#include <QFontDatabase>
class Captcha: public QWidget
{
     Q_OBJECT
public:
   explicit Captcha(QWidget* parent = nullptr, int length = 4);
    QString getCode() const;
    void refresh();
protected:
    void paintEvent(QPaintEvent * event) override;
    void mousePressEvent(QMouseEvent * event) override;
signals:
    void clicked();  // 点击验证码时发射此信号
private:
    QString generateCode();
    QColor getRandomColor(int min=0,int max=255);
    void drawDisturb(QPainter & painter);
    int getRandomInt(int min,int max);
     QString m_code;
    int m_codeLength;
};

#endif // CAPTCHA_H
