#ifndef SINGLETONDB_H
#define SINGLETONDB_H
#include "sqlite3.h"
#include<QDebug>
#include<QString>
#include<QMutex>
#include<QMutexLocker>

class SingletonDB
{
public:
    static SingletonDB * getInstance();
    /**
     * @brief execSql 执行sql语句 增删改
     * @param sql
     * @return 0-success else error
     */
    int execSql(QString sql);
    /**
     * @brief selectSql 执行查询语句
     * @param sql
     * @param qres 结果集char **
     * @param row 数据行 [没有包含表头]
     * @param col 数据列
     * @return 0-success else error
     */
    int selectSql(QString sql,char **&qres,int &row,int &col);
    /**
     * @brief insertVideo 插入视频记录到数据库
     * @param deviceId 设备ID
     * @param videoName 视频文件名
     * @param storagePath 存储路径
     * @param recordTime 录制时间
     * @param cover 封面路径（可为空）
     * @param totalFrames 总帧数（可为0）
     * @return 0-成功，其他-失败
     */
    int insertVideo(int deviceId, const QString &videoName, const QString &storagePath,
                   const QString &recordTime, const QString &cover = "", int totalFrames = 0);
    int getVideoFrameCount(const QString &storagePath);
private:
    SingletonDB();
    ~SingletonDB();
    static SingletonDB * instance;//静态成员
    sqlite3 *pdb;       //数据库操作指针
    QMutex m_mutex;     // 多线程安全锁（所有DB操作共用）
    // GC 机制
        class GC
        {
            public:~GC()
            {
                // 可以在这里销毁所有的资源，例如：db 连接、文件句柄等
                if (instance != nullptr) {
                    qDebug() << "Here destroy the m_pSingleton..." << endl;
                    delete instance;
                    instance = nullptr;
                }
            }
            static GC gc; // 用于释放单例
        };
};



#endif // SINGLETONDB_H
