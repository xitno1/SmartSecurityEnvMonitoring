#ifndef REGISTERWIDGET_H
#define REGISTERWIDGET_H
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>

class RegisterWidget : public QWidget {
    Q_OBJECT
public:
    explicit RegisterWidget(QWidget* parent = nullptr);
    void initUI();
    void initQss();

signals:
    void backToLogin();  // 返回登录信号
    void backToLogin(QString username);
private slots:
    void onRegisterClicked();
    void onCancelClicked();

private:
    QLineEdit* usernameEdit;
    QLineEdit* nicknameEdit;
    QLineEdit* passwordEdit;
    QLineEdit* confirmEdit;
    QPushButton* regBtn;
    QPushButton* cancelBtn;
    QToolButton* showPwdBtn;
    QToolButton* showConfirmBtn;
    QLabel* titleLabel;
    QLabel* userLabel;
    QLabel* nickLabel;
    QLabel* pwdLabel;
    QLabel* confirmLabel;
    bool isPwdVisible;
    bool isConfirmVisible;
};

#endif // REGISTERWIDGET_H
