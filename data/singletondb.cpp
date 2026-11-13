#include "SingletonDB.h"

SingletonDB ::GC SingletonDB::GC::gc;//类外初始化
//初始化
SingletonDB *SingletonDB::instance=nullptr;

SingletonDB *SingletonDB::getInstance()
{
    if(SingletonDB::instance==nullptr)
    {
        //类内可以调用私有成员函数
        SingletonDB::instance=new SingletonDB();
    }

    return SingletonDB::instance;
}

// 内部不加锁版本，供已加锁的公开方法调用
int SingletonDB::execSql(QString sql)
{
   QMutexLocker locker(&m_mutex);
   char * errmsg=nullptr;
   int res=sqlite3_exec(this->pdb,sql.toUtf8(),nullptr,nullptr,&errmsg);
   if(res!=SQLITE_OK)
   {
       qDebug()<<errmsg;
   }
   return res;
}

int SingletonDB::selectSql(QString sql, char **&qres, int &row, int &col)
{
    QMutexLocker locker(&m_mutex);
    char * errmsg=nullptr;
    int res=sqlite3_get_table(pdb,sql.toUtf8(),&qres,&row,&col,&errmsg);
    if(res!=SQLITE_OK)
    {
        qDebug()<<errmsg;
    }
    return res;
}

SingletonDB::SingletonDB()
{
    int res=sqlite3_open("./two.db",&pdb);
    if(res==SQLITE_OK)
    {
        qDebug()<<"open db success";
    }
    else {
        qDebug()<<sqlite3_errcode(pdb);
        qDebug()<<sqlite3_errmsg(pdb);
    }
}

SingletonDB::~SingletonDB()
{
    //释放类的指针成员
    sqlite3_close(pdb);
    qDebug()<<"db release";
}

int SingletonDB::insertVideo(int deviceId, const QString &videoName, const QString &storagePath,
                            const QString &recordTime, const QString &cover, int totalFrames)
{
    // 直接插入，不指定video_id（由数据库自动生成）
    QString sql = QString(
        "INSERT INTO video_file ("
        "device_id, video_name, storage_path, record_time, cover, total_frames"
        ") VALUES (%1, '%2', '%3', '%4', '%5', %6)"
    );

    // 处理单引号转义
    QString tempVideoName = videoName;
    tempVideoName.replace('\'', '\'\'');

    QString tempStoragePath = storagePath;
    tempStoragePath.replace('\'', '\'\'').replace('\\', '/');

    QString tempCover = cover;
    tempCover.replace('\'', '\'\'').replace('\\', '/');

    sql = sql.arg(deviceId)
             .arg(tempVideoName)
             .arg(tempStoragePath)
             .arg(recordTime)
             .arg(tempCover)
             .arg(totalFrames);

    // execSql 内部已加锁，无需在此重复加锁
    int execRes = execSql(sql);
    if (execRes != 0) {
        qDebug() << "数据库插入失败！SQL:" << sql << "错误码:" << execRes;
    }
    return execRes;
}

int SingletonDB::getVideoFrameCount(const QString &storagePath)
{
    // 处理路径中的单引号转义，避免SQL注入
    QString escapedPath = storagePath;
    escapedPath.replace('\'', '\'\'');
    QString sql = QString("SELECT total_frames FROM video_file WHERE storage_path = '%1'").arg(escapedPath);

    char **res = nullptr;
    int row = 0;   // 数据行数（不包含表头）
    int col = 0;   // 列数
    int queryRes = selectSql(sql, res, row, col);

    // 检查查询是否成功
    if (queryRes != SQLITE_OK)
    {
        qDebug() << "查询帧数失败，SQL:" << sql;
        return 0;
    }

    // 检查是否有数据行（row >= 1 表示有结果）
    if (row > 0 && col > 0) {
        // 结果集结构：res[0] = 表头（"total_frames"），res[1] = 第一行数据
        int frames = QString(res[1]).toInt();  // 正确读取第一行数据
        sqlite3_free_table(res);  // 释放结果集内存
        return frames;
    }

    // 无结果或异常情况
    sqlite3_free_table(res);  // 释放结果集内存
    return 0;
}
