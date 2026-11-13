#include "registerwidget.h"
#include "Control/usercontroller.h"
#include <QMessageBox>
#include <QFile>
#include <QDebug>

RegisterWidget::RegisterWidget(QWidget* parent)
    : QWidget(parent)
{
    this->isPwdVisible=false;
    this->isConfirmVisible=false;
    setWindowTitle("注册");
    resize(400, 350);
    initUI();
    initQss();
}

void RegisterWidget::initUI()
{

    titleLabel = new QLabel("注册", this);
    titleLabel->setObjectName("loginTitle");
    titleLabel->setStyleSheet("QLabel { font-family: \"微软雅黑\", \"Microsoft YaHei\"; font-size: 18px; font-weight: bold;"
                             " color: #2C3E50; text-align: left; padding: 4px 8px; }");
    titleLabel->move(150, 30);
    userLabel = new QLabel("账号：", this);
    userLabel->move(60, 70);
    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("6-10位数字或字母");
    usernameEdit->setFixedWidth(150);
    usernameEdit->move(120, 70);

    nickLabel = new QLabel("昵称：", this);
    nickLabel->move(60, 110);
    nicknameEdit = new QLineEdit(this);
    nicknameEdit->setPlaceholderText("6-10位中英文或数字");
    nicknameEdit->setFixedWidth(150);
    nicknameEdit->move(120, 110);
    pwdLabel = new QLabel("密码：", this);
    pwdLabel->move(60, 150);
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("6-10位数字或字母");
    passwordEdit->setFixedWidth(150);
    passwordEdit->move(120, 150);
    showPwdBtn = new QToolButton(this);
    showPwdBtn->setText("👁");
    showPwdBtn->setFixedSize(25, 25);
    showPwdBtn->move(270, 145);  // 位于密码框右侧
    connect(showPwdBtn, &QToolButton::clicked, [=]() {
        isPwdVisible = !isPwdVisible;
        passwordEdit->setEchoMode(isPwdVisible ? QLineEdit::Normal : QLineEdit::Password);
    });

    confirmLabel = new QLabel("确认密码：", this);
    confirmLabel->move(60, 190);
    confirmEdit = new QLineEdit(this);
    confirmEdit->setEchoMode(QLineEdit::Password);
    confirmEdit->setPlaceholderText("再次输入密码");
    confirmEdit->setFixedWidth(150);
    confirmEdit->move(120, 190);
    showConfirmBtn = new QToolButton(this);
    showConfirmBtn->setText("👁");
    showConfirmBtn->setFixedSize(25, 25);
    showConfirmBtn->move(270, 185);  // 位于确认密码框右侧
    connect(showConfirmBtn, &QToolButton::clicked, [=]() {
        isConfirmVisible = !isConfirmVisible;
        confirmEdit->setEchoMode(isConfirmVisible ? QLineEdit::Normal : QLineEdit::Password);
    });

    regBtn = new QPushButton("注册", this);
    regBtn->setObjectName("loginBtn");
    regBtn->move(80, 250);
    regBtn->setFixedWidth(100);

    cancelBtn = new QPushButton("取消", this);
    cancelBtn->setObjectName("regBtn");
    cancelBtn->move(220, 250);
    cancelBtn->setFixedWidth(100);

    connect(regBtn, &QPushButton::clicked, this, &RegisterWidget::onRegisterClicked);
    connect(cancelBtn, &QPushButton::clicked, this, &RegisterWidget::onCancelClicked);
}

void RegisterWidget::initQss() {

    QFile file(":/login.qss");
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "QSS文件打开失败：" << file.errorString();
        return;
    }
    QString styleSheet = QLatin1String(file.readAll());
    setStyleSheet(styleSheet);
    file.close();
}

void RegisterWidget::onRegisterClicked()
{
    QString username = usernameEdit->text().trimmed();
    QString nickname = nicknameEdit->text().trimmed();
    QString password = passwordEdit->text().trimmed();
    QString confirmPwd = confirmEdit->text().trimmed();


    int result = UserController::getInstance()->registerUser(username, nickname, password, confirmPwd);
    if (result == 0)
    {
        QMessageBox::information(this, "成功", "注册成功");
        emit backToLogin(username);  // 返回登录界面
        close();
    }
    else if (result == -1)
    {
        QMessageBox::warning(this, "错误", "账号或密码不能为空");
    }
    else if (result == -2)
    {
        QMessageBox::warning(this, "错误", "账号或密码格式错误（6-10位数字或字母）");
    }
    else if (result == -3)
    {
        QMessageBox::warning(this, "错误", "两次输入的密码不一致");
    }
    else if (result == -4)
    {
        QMessageBox::warning(this, "错误", "该账号已存在");
    }
    else
    {
        QMessageBox::warning(this, "错误", "注册失败，请重试");
    }
}

void RegisterWidget::onCancelClicked()
{
    emit backToLogin();  // 返回登录界面
    close();
}
