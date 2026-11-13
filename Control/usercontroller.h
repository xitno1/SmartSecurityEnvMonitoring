#ifndef USERCONTROLLER_H
#define USERCONTROLLER_H
#include <QString>
#include"model/usermodel.h"

class UserController
{
public:
    static UserController * getInstance();
    //处理注册
    int registerUser(const QString &username,const QString &nickname,const QString &password,const QString &confirmPwd);
    //处理登录
    int loginUser(const QString &username,const QString &password);
    //检查登录状态
    bool isLoggedIn() const;
    //获取当前用户名
    QString getCurrentUser() const;
    bool getIsLoggedIn() const;

    void logout();
private:
    UserController();
    static UserController * instance;
    bool m_isLoggedIn;
    QString m_currentUser;
};

#endif // USERCONTROLLER_H
