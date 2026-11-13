#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include "data/captcha.h"
#include "Control/usercontroller.h"
#include "view/registerwidget.h"
#include <QMessageBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

class LoginWidget: public QWidget
{
    Q_OBJECT
public:
    explicit LoginWidget(QWidget* parent = nullptr);
    void initUI();
    void initQss();
signals:
    void loginSuccess();
private slots:
    void onLoginClicked();
    void onRegisterClicked();
public slots:
    void showLogin(QString username);
    void showLogin();
private:
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QLineEdit* captchaEdit;
    Captcha* captchaWidget;
    QToolButton* showPwdBtn; // 密码显示/隐藏按钮
    bool isPwdVisible;
    QLabel* userLabel;
    QLabel* pwdLabel,*captchaLabel;
    QPushButton* loginBtn,* regBtn;
    RegisterWidget*regWidget=nullptr;

};

#endif // LOGINWIDGET_H
