#ifndef USERMODEL_H
#define USERMODEL_H
#include <QString>
#include<data/singletondb.h>
#include<QDateTime>
class UserModel
{
public:
    static UserModel* getInstance();
    //检查用户是否存在,用来判断注册的
    int checkUsernameExists(const QString & username);
    //插入新用户，注册成功的
    int insertUser(const QString &username,const QString &nickname,const QString & password);
    //验证用户名密码查找用户，用来判断登录的
    int verifyUser(const QString & username,const QString & password);
private:
    UserModel();
    static UserModel* instance;
};

#endif // USERMODEL_H
