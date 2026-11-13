#include "usermodel.h"
UserModel* UserModel::instance = nullptr;

UserModel *UserModel::getInstance()
{
    if(UserModel::instance==nullptr)
    {
        UserModel::instance=new UserModel();
    }
    return UserModel::instance;
}

int UserModel::checkUsernameExists(const QString &username)
{
    QString sql=QString("select * from user where username='%1'").arg(username);
    char **qres=nullptr;
    int row=0,col=0;
    int res=SingletonDB::getInstance()->selectSql(sql,qres,row,col);
    if(res!=0)
    {
        return -1;//查询失败
    }
    return row>0?1:0;   //1-存在，0-不存在
}

int UserModel::insertUser(const QString &username, const QString &nickname, const QString &password)
{
    //获取当前时间作为创建时间
    QString createTime=QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString idsql="select max(user_id) from user";
    char **qres=nullptr;
    int row=0,col=0;
    int userId=1;
    int res=SingletonDB::getInstance()->selectSql(idsql,qres,row,col);
    if(res==0)
    {
        userId=(qres[1]?QString(qres[1]).toInt():0)+1;
    }
    else {
        return 0;
    }
    sqlite3_free_table(qres);
    QString sql = QString(
           "INSERT INTO user (user_id, username, nickname, password, create_time) "
           "VALUES (%1, '%2', '%3', '%4', '%5')"
       ).arg(userId)
        .arg(username)
        .arg(nickname)
        .arg(password)
        .arg(createTime);
    return SingletonDB::getInstance()->execSql(sql);//0-成功，else 失败
}

int UserModel::verifyUser(const QString &username, const QString &password)
{
    QString sql=QString("select * from user where username='%1' and password='%2'").arg(username).arg(password);
    char ** qres=nullptr;
    int row=0,col=0;
    int res=SingletonDB::getInstance()->selectSql(sql,qres,row,col);
    if(res!=0)
    {
        return -1;//查询失败
    }
    return row>0?1:0;//1-验证成功 ，0-验证失败
}

UserModel::UserModel()
{

}
