#ifndef VIDEOPLAYBACKWIDGET_H
#define VIDEOPLAYBACKWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QListWidget>
#include <QDateEdit>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QHeaderView>
#include "thread/playthread.h"
#include "data/singletondb.h"
#include"Control/usercontroller.h"
class VideoPlaybackWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoPlaybackWidget(QWidget *parent = nullptr);
    ~VideoPlaybackWidget();

    static const int PAGE_SIZE = 10;  // 每页显示10条记录
    void dinshiqi();

signals:
    void logout();  // 退出登录信号
    void backToMain();  // 返回主界面信号
private slots:
    // 视频控制相关
    void onPlayPauseClicked();
    void onSpeedChanged(int index);
    void onProgressChanged(int value);  // 进度条拖动事件
    void updateVideoFrame(QImage img);
    void updatePlayProgress(qint64 pos);
    void onVideoFinished();
    void onFrameInfoReceived(int currentFrame, int totalFrames, double fps);  // 新增：接收帧信息
    void onFrameProgress(int currentFrame, int totalFrames, double fps);
    // 列表相关
    void loadVideoList();

    // 导出相关
    void onExportClicked();

    // 退出登录
    void onLogoutClicked();

private:
    // 界面初始化
    void initUI();
    void initConnections();

    QList<QMap<QString, QString>> getVideoData(const QDate &date, int page, int pageSize = PAGE_SIZE);
    int getTotalVideoCount(const QDate &date);

    // 播放控制
    void playVideo(const QString &filePath);
    void stopCurrentVideo();
    void loadNextVideo();
    void markVideoAsInvalid(const QString &path);
    void removeInvalidVideoFromList(const QString &path);
    void findAndPlayFirstAvailableVideo();
    bool isVideoFileReady(const QString &path);

    // 顶部组件
    QLabel *logoLabel;
    QLabel *timeLabel;
    QLabel *userLabel;
    QPushButton *logoutBtn;
    QPushButton *backBtn;
    // 左侧播放区域
    QLabel *videoDisplayLabel;
    QSlider *progressSlider;
    QPushButton *playPauseBtn;
    QPushButton *stopBtn;
    QComboBox *speedCombo;
    QPushButton *exportBtn;

    // 右侧列表区域
    QDateEdit *dateEdit;
    QListWidget *videoListWidget;
    QLabel *pageLabel;
    QPushButton *prevBtn;
    QPushButton *nextBtn;
    QPushButton *queryBtn;

    // 播放相关
    PlayThread *playThread=nullptr;
    bool isPlaying;
    QString currentVideoPath;
    int currentPage;
    int totalPages;
    QDate currentQueryDate;
    QList<QMap<QString, QString>> currentVideoList;
    int currentVideoIndex;
    QComboBox *formatCombo; // 导出格式选择下拉框

    // 定时器
    QTimer *timeUpdateTimer;
    QTimer *dsq;
    bool hasAutoPlayedFirstVideo;
    bool m_isUserSeeking;  // 标记用户是否正在拖拽进度条
};

#endif // VIDEOPLAYBACKWIDGET_H
