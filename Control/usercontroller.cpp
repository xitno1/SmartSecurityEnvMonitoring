#include "usercontroller.h"
#include <QCryptographicHash>
#include <QRegularExpression>
UserController* UserController::instance = nullptr;

UserController::UserController()
{
    this->m_isLoggedIn=false;
}


bool UserController::getIsLoggedIn() const
{
    return m_isLoggedIn;
}

UserController *UserController::getInstance()
{
    if(UserController::instance==nullptr)
    {
        UserController::instance=new UserController();
    }
    return UserController::instance;
}
//MD5加密
QString encryptPassword(const QString& password)
{
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5);
    return hash.toHex();
}

//验证账号/密码格式（6-10位数字或字母）
bool isValidFormat(const QString &str)
{
    QRegularExpression regex("^[A-Za-z0-9]{6,10}$");
    return regex.match(str).hasMatch();
    return  regex.match(str).hasMatch();
}

int UserController::registerUser(const QString &username, const QString &nickname, const QString &password, const QString &confirmPwd)
{
    if(username.isEmpty()||password.isEmpty())
    {
        return -1;//账号或密码为空
    }
    if(!isValidFormat(username)||!isValidFormat(password))
    {
        return -2;//格式错误
    }
    if(password!=confirmPwd)
    {
        return -3;//密码不一致
    }
    if(UserModel::getInstance()->checkUsernameExists(username)==1)
    {
        return -4;//用户已存在
    }
    if(UserModel::getInstance()->insertUser(username,nickname,encryptPassword(password))==0)
    {
        return 0;//注册成功；
    }
    return -5;//注册失败
}

int UserController::loginUser(const QString &username, const QString &password)
{
   if(username.isEmpty()||password.isEmpty())
   {
       return -1;//账号或密码为空
   }
   int res=UserModel::getInstance()->verifyUser(username,encryptPassword(password));
   if(res==1)
   {
       m_isLoggedIn=true;
       m_currentUser=username;
       return 0;//登录成功
   }
   else if(res==0)
   {
       return -2;//账号或密码错误
   }
   return -3;//登录失败
}

bool UserController::isLoggedIn() const
{
     return m_isLoggedIn;
}

QString UserController::getCurrentUser() const
{
     return m_currentUser;
}

void UserController::logout()
{
    m_isLoggedIn = false;
    m_currentUser = "";
    qDebug() << "用户已登出";
}
