#include "captcha.h"
#include <QPainter>
#include <QTime>
#include <QMouseEvent>
#include <QFont>

// 初始化随机数种子（确保只初始化一次）
void initRandomSeed()
{
    static bool initialized = false;
    if (!initialized)
    {
        qsrand(QTime::currentTime().msecsSinceStartOfDay()); // 用当前时间作为种子
        initialized = true;
    }
}

Captcha::Captcha(QWidget* parent, int length)
    : QWidget(parent), m_codeLength(length)
{
    initRandomSeed(); // 初始化随机数
    setFixedSize(120, 40); // 设置验证码控件大小
    setCursor(Qt::PointingHandCursor); // 鼠标悬停显示手型，提示可点击
    m_code = generateCode(); // 生成初始验证码
}

QString Captcha::generateCode() {
    // 验证码字符集（排除易混淆的0/O、1/l等字符）
    const QString chars = "0123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghjkmnpqrstuvwxyz";
    QString code;
    for (int i = 0; i < m_codeLength; ++i)
    {
        // 生成随机索引（0到字符集长度-1）
        int index = qrand() % chars.length();
        code.append(chars.at(index));
    }
    return code;
}

QColor Captcha::getRandomColor(int min, int max)
{
    // 生成指定范围内的随机RGB颜色
    int r = min + qrand() % (max - min + 1);
    int g = min + qrand() % (max - min + 1);
    int b = min + qrand() % (max - min + 1);
    return QColor(r, g, b);
}

int Captcha::getRandomInt(int min, int max)
{
    // 生成指定范围内的随机整数
    return min + qrand() % (max - min + 1);
}

void Captcha::drawDisturb(QPainter& painter)
{
    // 绘制干扰线（5条）
    painter.setPen(QPen(getRandomColor(100, 200), 1)); // 线条颜色较浅，不影响字符识别
    for (int i = 0; i < 5; ++i)
    {
        painter.drawLine(
            getRandomInt(0, width()),  // 起点x
            getRandomInt(0, height()), // 起点y
            getRandomInt(0, width()),  // 终点x
            getRandomInt(0, height())  // 终点y
        );
    }

    // 绘制干扰点（50个）
    for (int i = 0; i < 50; ++i)
    {
        painter.setPen(QPen(getRandomColor(50, 200), 1));
        painter.drawPoint(
            getRandomInt(0, width()),
            getRandomInt(0, height())
        );
    }
}

void Captcha::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event); // 忽略未使用的参数

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 抗锯齿，使图形更平滑

    // 绘制背景（浅色调，与字符区分）
    painter.fillRect(rect(), getRandomColor(220, 255));

    // 绘制干扰元素（线和点）
    drawDisturb(painter);

    // 绘制验证码字符
    painter.setFont(QFont("SimHei", 18, QFont::Bold)); // 使用黑体，加粗
    int charWidth = width() / m_codeLength; // 每个字符的平均宽度

    for (int i = 0; i < m_codeLength; ++i)
    {
        // 随机字符颜色（深色，与背景对比）
        painter.setPen(getRandomColor(0, 150));

        // 保存当前绘图状态
        painter.save();

        // 字符位置微调（增加随机性）
        int x = i * charWidth + getRandomInt(3, 8);
        int y = height() / 2 + getRandomInt(0, 10);

        // 随机旋转角度（-30到30度）
        painter.translate(x, y); // 移动坐标原点到字符绘制位置
        painter.rotate(getRandomInt(-30, 30));

        // 绘制单个字符
        painter.drawText(0, 0, QString(m_code[i]));

        // 恢复绘图状态
        painter.restore();
    }
}

void Captcha::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
            refresh();
            emit clicked();  // 发射点击信号

    }
    QWidget::mousePressEvent(event);
}

void Captcha::refresh()
{
    m_code = generateCode(); // 重新生成验证码
    update(); // 触发重绘事件，更新显示
}
QString Captcha::getCode() const
{
    return m_code;
}
