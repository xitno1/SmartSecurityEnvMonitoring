#include "loginwidget.h"


LoginWidget::LoginWidget(QWidget *parent): QWidget(parent)
{
    this->isPwdVisible=false;
    this->setWindowTitle("登录");
    this->resize(400,300);
    this->initUI();
    this->initQss();
}

void LoginWidget::initUI()
{
    // 登录标题
    QLabel *loginLabel = new QLabel("登录", this);
    loginLabel->setObjectName("loginTitle");
    loginLabel->setStyleSheet("QLabel { font-family: \"微软雅黑\", \"Microsoft YaHei\"; font-size: 18px; font-weight: bold;"
                              " color: #2C3E50; text-align: left; padding: 4px 8px; }");
    loginLabel->move(150, 30);

    // 账号相关
    this->userLabel = new QLabel("账号：", this);
    this->userLabel->move(60, 70);
    this->usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("6-10位数字或字母");
    usernameEdit->setFixedWidth(150);
    this->usernameEdit->move(120, 70);

    // 密码相关
    this->pwdLabel = new QLabel("密码：", this);
    this->pwdLabel->move(60, 110);
    this->passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("6-10位数字或字母");
    passwordEdit->setFixedWidth(150);
    this->passwordEdit->move(120, 110);

    // 显示密码按钮
    showPwdBtn = new QToolButton(this);
    showPwdBtn->setText("👁");
    showPwdBtn->setFixedSize(25, 25);
    showPwdBtn->move(270, 105); // 密码框右边
    connect(showPwdBtn, &QToolButton::clicked, [=]() {
        isPwdVisible = !isPwdVisible;
        passwordEdit->setEchoMode(isPwdVisible ? QLineEdit::Normal : QLineEdit::Password);
    });

    // 验证码相关
    this->captchaLabel = new QLabel("验证码", this);
    this->captchaLabel->move(60, 150);
    this->captchaEdit = new QLineEdit(this);
    this->captchaEdit->setFixedWidth(120);
    this->captchaEdit->move(120, 150);
    captchaWidget = new Captcha(this);
    this->captchaWidget->move(250, 140);
    connect(captchaWidget, &Captcha::clicked, captchaWidget, &Captcha::refresh);

    // 按钮相关
    this->loginBtn = new QPushButton("登录", this);
    this->loginBtn->move(80, 200);
    this->loginBtn->setObjectName("loginBtn");
    this->regBtn = new QPushButton("注册", this);
    this->regBtn->setObjectName("regBtn");
    this->regBtn->move(250, 200);
    connect(loginBtn, &QPushButton::clicked, this, &LoginWidget::onLoginClicked);
    connect(regBtn, &QPushButton::clicked, this, &LoginWidget::onRegisterClicked);
}

void LoginWidget::initQss()
{
    QFile file(":/login.qss");  // 确保资源文件路径正确
    qDebug() << "QSS文件打开成功：";
   if (!file.open(QFile::ReadOnly | QFile::Text)) {  // 增加错误判断
       qDebug() << "QSS文件打开失败：" << file.errorString();
       return;
   }
   QString styleSheet = QLatin1String(file.readAll());
   this->setStyleSheet(styleSheet);
   file.close();  // 确保文件关闭
}

void LoginWidget::onLoginClicked()
{
    QString username = usernameEdit->text();
    QString password = passwordEdit->text();
    QString inputCaptcha = captchaEdit->text().toUpper(); // 不区分大小写

    // 验证码校验
    if (inputCaptcha != captchaWidget->getCode().toUpper())
    {
        QMessageBox::warning(this, "错误", "验证码错误");
        captchaWidget->refresh();
        return;
    }

    // 登录校验
    int result = UserController::getInstance()->loginUser(username, password);
    if (result == 0)
    {
        QMessageBox::information(this, "成功", "登录成功");
        emit loginSuccess();
        close();
    }
    else if (result == -1)
    {
        QMessageBox::warning(this, "错误", "账号或密码不能为空");
    }
    else if (result == -2)
    {
        QMessageBox::warning(this, "错误", "账号或密码不正确");
    }
}

void LoginWidget::onRegisterClicked()
{
    if (regWidget != nullptr)
    {
       regWidget->deleteLater();  // 延迟销毁，避免立即访问问题
       regWidget = nullptr;
    }
     regWidget = new RegisterWidget(nullptr);
     connect(regWidget,SIGNAL(backToLogin()),this,SLOT(showLogin()));
     connect(regWidget,SIGNAL(backToLogin(QString)),this,SLOT(showLogin(QString)));
     regWidget->show();
     this->hide();
}

void LoginWidget::showLogin(QString username)
{
    this->usernameEdit->setText(username);
    this->showLogin();
}

void LoginWidget::showLogin()
{
    if (regWidget != nullptr)
    {
        regWidget->deleteLater();
        regWidget = nullptr;
    }
    this->show();
}


