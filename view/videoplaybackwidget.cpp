#include "videoplaybackwidget.h"
#include "data/singletondb.h"
#include "thread/playthread.h"
#include "thread/trancecode.h"
#include <QThread>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDateEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QDateTime>
#include <QFileInfo>
#include <sqlite3.h>
#include <QDebug>

VideoPlaybackWidget::VideoPlaybackWidget(QWidget *parent)
    : QWidget(parent), isPlaying(false), currentPage(1), currentVideoIndex(-1),
      playThread(nullptr), m_isUserSeeking(false)
{
    this->setWindowTitle("视频回放");
    this->resize(1280, 720);
    this->initUI();
    this->initConnections();
    //不能在构造函数中加在数据
    this->dsq = nullptr;
    timeUpdateTimer = nullptr;
    this->setAttribute(Qt::WA_DeleteOnClose);
}

void VideoPlaybackWidget::dinshiqi()
{
    this->loadVideoList();
    // 重置自动播放标志
     hasAutoPlayedFirstVideo = false;
    if(!this->dsq)
    {
        dsq = new QTimer(this);
        connect(dsq, &QTimer::timeout, this, [this]() {
            if (!currentVideoList.isEmpty())
            {

                // 自动播放第一个视频
                findAndPlayFirstAvailableVideo();
            }
        });
        dsq->start(500);
    }

    if(!timeUpdateTimer)
    {
        // 初始化定时器更新系统时间
        timeUpdateTimer = new QTimer(this);
        connect(timeUpdateTimer, &QTimer::timeout, this, [this]() {
            timeLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
        });
        timeUpdateTimer->start(1000);
    }

}

void VideoPlaybackWidget::findAndPlayFirstAvailableVideo()
{
    // 先检查是否有视频列表
    if (currentVideoList.isEmpty())
    {
        videoDisplayLabel->setText("没有找到可用的视频文件");
        qDebug() << "视频列表为空";
        return;
    }

    // 检查是否已经在播放某个视频
    if (playThread != nullptr && playThread->isRunning() && isPlaying)
    {
        qDebug() << "已有视频正在播放，跳过自动播放第一个视频";
        return;
    }

    // 检查是否已经自动播放过第一个视频
    if (hasAutoPlayedFirstVideo)
    {
        qDebug() << "已经自动播放过第一个视频，跳过";
        return;
    }

    // 记录可用视频数量
    int availableCount = 0;

    // 查找第一个可用视频
    for (int i = 0; i < currentVideoList.count(); ++i)
    {
        QString path = currentVideoList[i]["path"];
        qDebug() << "检查视频" << i << ":" << path;

        if (isVideoFileReady(path))
        {
            qDebug() << "找到可用视频，播放第" << i << "个:" << path;
            currentVideoIndex = i;
            currentVideoPath = path;

            // 先选中列表项
            videoListWidget->setCurrentRow(i);

            // 显示准备信息
            videoDisplayLabel->setText("准备播放视频...");

            // 延迟一小会儿再播放，确保UI更新完成
            QTimer::singleShot(100, this, [this, path]() {
                playVideo(path);
            });

            isPlaying = true;
            playPauseBtn->setText("暂停");
            hasAutoPlayedFirstVideo = true;  // 标记已处理第一次播放

            // 停止定时器，避免重复尝试
            if (dsq) {
                dsq->stop();
            }
            return;
        }
        else
        {
            qDebug() << "跳过不可用视频:" << path;
        }
    }

    // 没有找到可用视频，也标记为已尝试
    hasAutoPlayedFirstVideo = true;
}

// 进度更新槽函数
void VideoPlaybackWidget::onFrameProgress(int currentFrame, int totalFrames, double fps)
{
    if (totalFrames <= 0) return;
    // 计算进度百分比（当前帧 / 实际总帧）
    int progress = (currentFrame * 100) / totalFrames;
    progressSlider->setValue(progress);  // 更新进度条
}

VideoPlaybackWidget::~VideoPlaybackWidget()
{
    stopCurrentVideo();
    if (timeUpdateTimer)
    {
        timeUpdateTimer->stop();
        delete timeUpdateTimer;
        timeUpdateTimer = nullptr;
    }

    if (playThread)
    {
        disconnect(playThread, nullptr, this, nullptr); // 断开所有信号
    }
}

void VideoPlaybackWidget::initUI()
{
    // 主布局（上中下结构）
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(5);

    // 顶部导航栏
    QWidget *topNavWidget = new QWidget();
    topNavWidget->setStyleSheet("background-color: #0F3460; height: 60px;");
    QHBoxLayout *topLayout = new QHBoxLayout(topNavWidget);
    topLayout->setContentsMargins(10, 5, 10, 5);

    // Logo
    logoLabel = new QLabel();
    logoLabel->setPixmap(QPixmap(":/images/logo.png").scaled(40, 40, Qt::KeepAspectRatio));
    logoLabel->setMinimumSize(50, 50);

    // 标题
    QLabel *titleLabel = new QLabel("视频回放");
    titleLabel->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");

    // 系统时间
    timeLabel = new QLabel();
    timeLabel->setStyleSheet("color: white; font-size: 14px;");

    // 用户信息
    userLabel = new QLabel("当前用户: admin");
    userLabel->setStyleSheet("color: white; font-size: 14px;");

    // 退出按钮
    logoutBtn = new QPushButton("退出");
    logoutBtn->setStyleSheet("background-color: #1A73E8; color: white; padding: 5px 15px; border-radius: 4px;");

    backBtn = new QPushButton("返回");
    backBtn->setStyleSheet("background-color: #1A73E8; color: white; padding: 5px 15px; border-radius: 4px;");

    topLayout->addWidget(logoLabel);
    topLayout->addSpacing(10);
    topLayout->addWidget(titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(timeLabel);
    topLayout->addSpacing(20);
    topLayout->addWidget(userLabel);
    topLayout->addSpacing(20);
    topLayout->addWidget(backBtn);
    topLayout->addSpacing(20);
    topLayout->addWidget(logoutBtn);

    // 中间区域
    QWidget *midWidget = new QWidget();
    QHBoxLayout *midLayout = new QHBoxLayout(midWidget);
    midLayout->setContentsMargins(5, 5, 5, 5);
    midLayout->setSpacing(5);

    // 左侧播放区域
    QWidget *leftPlayWidget = new QWidget();
    leftPlayWidget->setStyleSheet("background-color: #2D2D2D;");
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPlayWidget);
    leftLayout->setContentsMargins(5, 5, 5, 5);

    // 视频显示
    videoDisplayLabel = new QLabel();
    videoDisplayLabel->setAlignment(Qt::AlignCenter);
    videoDisplayLabel->setStyleSheet("background-color: black;");
    videoDisplayLabel->setMinimumHeight(480);

    // 进度条
    progressSlider = new QSlider(Qt::Horizontal);
    progressSlider->setStyleSheet("QSlider::groove:horizontal { background: #5c5c5c; height: 6px; border-radius: 3px; }"
                                  "QSlider::handle:horizontal { background: #1A73E8; width: 16px; height: 16px; margin: -5px 0; border-radius: 8px; }");

    leftLayout->addWidget(videoDisplayLabel);
    leftLayout->addWidget(progressSlider);

    // 右侧列表区域
    QWidget *rightListWidget = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightListWidget);
    rightLayout->setContentsMargins(5, 5, 5, 5);

    // 日期选择和查询
    QHBoxLayout *dateLayout = new QHBoxLayout();
    QLabel *dateLabel = new QLabel("选择日期:");
    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setDisplayFormat("yyyy-MM-dd");
    queryBtn = new QPushButton("查询");
    dateLayout->addWidget(dateLabel);
    dateLayout->addWidget(dateEdit);
    dateLayout->addWidget(queryBtn);

    // 视频列表
    videoListWidget = new QListWidget();
    videoListWidget->setStyleSheet("QListWidget{border:1px solid #ccc;background:white;} "
                                   "QListWidget::item{height:80px;border-bottom:1px solid #eee;} "
                                   "QListWidget::item:selected{background:#e3f2fd;}");
    videoListWidget->setSpacing(2);  // 项之间的间距
    videoListWidget->setIconSize(QSize(100, 60));  // 封面图片大小
    videoListWidget->setViewMode(QListWidget::ListMode);  // 列表模式
    videoListWidget->setResizeMode(QListWidget::Adjust);
    videoListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 分页控制
    QHBoxLayout *pageLayout = new QHBoxLayout();
    prevBtn = new QPushButton("上一页");
    nextBtn = new QPushButton("下一页");
    pageLabel = new QLabel("第 1 页");
    pageLayout->addStretch();
    pageLayout->addWidget(prevBtn);
    pageLayout->addWidget(pageLabel);
    pageLayout->addWidget(nextBtn);
    pageLayout->addStretch();

    rightLayout->addLayout(dateLayout);
    rightLayout->addWidget(videoListWidget);  // 添加列表控件
    rightLayout->addLayout(pageLayout);

    // 设置中间区域比例
    midLayout->addWidget(leftPlayWidget,7);
    midLayout->addWidget(rightListWidget, 3);

    // 3. 底部控制区
    QWidget *bottomControlWidget = new QWidget();
    bottomControlWidget->setStyleSheet("background-color: #0F3460; height: 50px;");
    QHBoxLayout *controlLayout = new QHBoxLayout(bottomControlWidget);
    controlLayout->setContentsMargins(10, 5, 10, 5);
    controlLayout->setSpacing(20);

    playPauseBtn = new QPushButton("播放");
    stopBtn = new QPushButton("停止");
    speedCombo = new QComboBox();
    speedCombo->addItems({"0.5x", "1.0x", "1.5x", "2.0x"});
    speedCombo->setCurrentIndex(1);

    formatCombo = new QComboBox();
    formatCombo->addItems({"MP4", "AVI", "MKV", "FLV"}); // 支持的格式列表
    formatCombo->setStyleSheet("background-color: white; padding: 5px;");

    exportBtn = new QPushButton("导出");

    // 设置按钮样式
    QString btnStyle = "background-color: #1A73E8; color: white; padding: 5px 15px; border-radius: 4px;";
    playPauseBtn->setStyleSheet(btnStyle);
    stopBtn->setStyleSheet(btnStyle);
    exportBtn->setStyleSheet(btnStyle);
    speedCombo->setStyleSheet("background-color: white; padding: 5px;");

    controlLayout->addWidget(playPauseBtn);
    controlLayout->addWidget(stopBtn);
    controlLayout->addWidget(new QLabel("播放速度:"));
    controlLayout->addWidget(speedCombo);
    controlLayout->addSpacing(20);
    controlLayout->addWidget(new QLabel("导出格式:")); // 标签
    controlLayout->addWidget(formatCombo); // 格式下拉框
    controlLayout->addStretch();
    controlLayout->addWidget(exportBtn);

    // 添加到主布局
    mainLayout->addWidget(topNavWidget);
    mainLayout->addWidget(midWidget, 1);
    mainLayout->addWidget(bottomControlWidget);

    setLayout(mainLayout);
}

void VideoPlaybackWidget::initConnections()
{
    connect(dateEdit, &QDateEdit::dateChanged, this, [this]() {
        currentPage = 1;
        loadVideoList();
    });

    connect(queryBtn, &QPushButton::clicked, this, [this]() {
        currentPage = 1;
        loadVideoList();
    });

    connect(prevBtn, &QPushButton::clicked, this, [this]() {
        if (currentPage > 1) {
            currentPage--;
            loadVideoList();
        }
    });

    connect(nextBtn, &QPushButton::clicked, this, [this]() {
        int totalCount = getTotalVideoCount(dateEdit->date());
        int totalPages = (totalCount + PAGE_SIZE - 1) / PAGE_SIZE;
        if (currentPage < totalPages)
        {
            currentPage++;
            loadVideoList();
        }
    });

    connect(backBtn, &QPushButton::clicked, this, [this]() {
        // 停止当前视频播放（确保线程完全停止）
        stopCurrentVideo();

        // 断开所有信号连接，避免回调到已销毁的对象
        if (playThread) {
            disconnect(playThread, &PlayThread::sendimg, this, &VideoPlaybackWidget::updateVideoFrame);
            disconnect(playThread, &PlayThread::sendPosition, this, &VideoPlaybackWidget::updatePlayProgress);
            disconnect(playThread, &PlayThread::sendFrameInfo, this, &VideoPlaybackWidget::onFrameProgress);
        }

        // 延迟隐藏，确保资源释放完成
        QTimer::singleShot(100, this, [this]() {
            this->hide();
            emit backToMain();
        });
    });

    // 列表项点击事件
    connect(videoListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        if (!item) return;

        QString videoPath = item->data(Qt::UserRole).toString();
        qDebug() << "点击播放视频:" << videoPath;
        if (!videoPath.isEmpty())
        {
            // 检查是否是图片文件
            QFileInfo fileInfo(videoPath);
            if (fileInfo.suffix().toLower() == "jpg" ||
                fileInfo.suffix().toLower() == "jpeg" ||
                fileInfo.suffix().toLower() == "png")
            {
                qDebug() << "错误：点击的是图片文件，不是视频文件:" << videoPath;
                QMessageBox::warning(this, "错误", "选择的是图片文件，请选择视频文件");
                return;
            }


            currentVideoPath = videoPath;
            currentVideoIndex = videoListWidget->row(item);

            // 停止当前播放，然后播放新视频
            stopCurrentVideo();
            QTimer::singleShot(50, this, [this, videoPath]() {
                playVideo(videoPath);
                isPlaying = true;
                playPauseBtn->setText("暂停");
            });
        }
    });



       // 播放/暂停按钮
       connect(playPauseBtn, &QPushButton::clicked, this, &VideoPlaybackWidget::onPlayPauseClicked);
       // 停止按钮
       connect(stopBtn, &QPushButton::clicked, this, [this]() {
           stopCurrentVideo();
           isPlaying = false;
           playPauseBtn->setText("播放");
       });
       // 速度选择
       connect(speedCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &VideoPlaybackWidget::onSpeedChanged);
       // 进度条拖拽中实时跳转；释放时最终跳转
       // isSliderDown() 直接查询硬件状态，比信号标记更可靠
       connect(progressSlider, &QSlider::sliderMoved, this, &VideoPlaybackWidget::onProgressChanged);
       connect(progressSlider, &QSlider::sliderReleased, this, [this]() {
           onProgressChanged(progressSlider->value());
       });
       // 导出按钮
       connect(exportBtn, &QPushButton::clicked, this, &VideoPlaybackWidget::onExportClicked);
       // 退出按钮
       connect(logoutBtn, &QPushButton::clicked, this, &VideoPlaybackWidget::onLogoutClicked);
}

void VideoPlaybackWidget::loadVideoList()
{
    QDate currentDate = dateEdit->date();
    qDebug() << "开始加载视频列表，日期:" << currentDate.toString("yyyy-MM-dd");

    currentVideoList = getVideoData(currentDate, currentPage, PAGE_SIZE);
    int totalCount = getTotalVideoCount(currentDate);
    totalPages = (totalCount + PAGE_SIZE - 1) / PAGE_SIZE;

    qDebug() << "分页信息 - 当前页:" << currentPage << "总页数:" << totalPages
             << "数据条数:" << currentVideoList.count();

    // 清空列表
    videoListWidget->clear();
    videoListWidget->setStyleSheet("QListWidget { font-size: 14px; border: 1px solid #ccc; color: #333; background-color: white; }"
                                  "QListWidget::item { padding: 5px; }"
                                  "QListWidget::item:selected { background-color: #e0e0e0; }");

    // 填充列表项
    for (int i = 0; i < currentVideoList.count(); ++i)
    {
        QMap<QString, QString> video = currentVideoList[i];

        // 检查必要数据是否存在
        if (video["name"].isEmpty() || video["path"].isEmpty())
        {
            qDebug() << "跳过无效视频数据，索引:" << i;
            continue;
        }

        // 检查文件是否存在
        QFileInfo fileInfo(video["path"]);
        bool fileExists = fileInfo.exists();

        QListWidgetItem *item = new QListWidgetItem();
        QString coverPath = video["cover"];
        QPixmap coverPix;

        if (!fileExists)
        {
            qDebug() << "视频文件不存在:" << video["path"];
            // 使用不同的图标表示文件不存在
            coverPix = QPixmap(":/images/warning.png").scaled(120, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            if (coverPix.isNull())
            {
                coverPix = QPixmap(120, 80);
                coverPix.fill(Qt::red);
            }
        }
        else if (!coverPath.isEmpty() && QFile::exists(coverPath))
        {
            coverPix = QPixmap(coverPath).scaled(120, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        else
        {
            // 使用默认图标
            coverPix = QPixmap(":/images/cover.png").scaled(120, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            if (coverPix.isNull())
            {
                coverPix = QPixmap(120, 80);
                coverPix.fill(fileExists ? Qt::lightGray : Qt::red);
            }
        }

        item->setIcon(QIcon(coverPix));

        // 根据文件是否存在设置不同的文本
        if (!fileExists)
        {
            item->setText(QString("⚠ %1\n文件不存在\n%2").arg(video["name"]).arg(video["time"]));
            item->setForeground(Qt::red);
            item->setToolTip("视频文件不存在或已被移动: " + video["path"]);
        }
        else
        {
            item->setText(QString("%1\n%2").arg(video["name"]).arg(video["time"]));
        }

        item->setSizeHint(QSize(140, 120));
        item->setData(Qt::UserRole, video["path"]);
        item->setData(Qt::UserRole + 1, fileExists); // 存储文件是否存在标志

        videoListWidget->addItem(item);
        qDebug() << "添加列表项:" << video["name"] << "文件存在:" << fileExists;
    }


    // 更新分页信息
    pageLabel->setText(QString("第 %1/%2 页").arg(currentPage).arg(totalPages));
    prevBtn->setEnabled(currentPage > 1);
    nextBtn->setEnabled(currentPage < totalPages);
    qDebug() << "列表加载完成，总项数:" << videoListWidget->count();

    if (videoListWidget->count() == 0)
    {
        QListWidgetItem *item = new QListWidgetItem("该日期没有视频数据");
        item->setTextAlignment(Qt::AlignCenter);
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
        videoListWidget->addItem(item);
    }
}

QList<QMap<QString, QString>> VideoPlaybackWidget::getVideoData(const QDate &date, int page, int pageSize)
{
       QList<QMap<QString, QString>> result;
       char **qres = nullptr;
       int row = 0, col = 0;
       QString dateStr = date.toString("yyyy-MM-dd");
       int offset = (page - 1) * pageSize;

       QString sql = QString(
           "SELECT video_name, "
           "storage_path as video_path, "
           "record_time, cover as cover_path "
           "FROM video_file "
           "WHERE DATE(record_time) = '%1' "
           "ORDER BY record_time DESC "
           "LIMIT %2 OFFSET %3"
       ).arg(dateStr).arg(pageSize).arg(offset);

       qDebug() << "执行SQL:" << sql;

       int res = SingletonDB::getInstance()->selectSql(sql, qres, row, col);
       if (res != SQLITE_OK)
       {
           qDebug() << "查询视频列表失败，错误码:" << res;
           if (qres) sqlite3_free_table(qres);
           return result;
       }

       qDebug() << "查询成功 - 行数:" << row << "列数:" << col;

       for (int i = 0; i < row; ++i)
       {
           QMap<QString, QString> video;
           int baseIndex = (i + 1) * col;

           video["name"] = qres[baseIndex + 0] ? QString::fromUtf8(qres[baseIndex + 0]) : "";
           video["path"] = qres[baseIndex + 1] ? QString::fromUtf8(qres[baseIndex + 1]) : "";  // 视频路径
           video["time"] = qres[baseIndex + 2] ? QString::fromUtf8(qres[baseIndex + 2]) : "";
           video["cover"] = qres[baseIndex + 3] ? QString::fromUtf8(qres[baseIndex + 3]) : ""; // 封面路径

           qDebug() << "视频数据" << i << ":"
                    << "name=" << video["name"]
                    << "path=" << video["path"]
                    << "time=" << video["time"]
                    << "cover=" << video["cover"];

           // 验证文件是否存在
           QFileInfo videoFile(video["path"]);
           if (!videoFile.exists())
           {
               qDebug() << "警告: 视频文件不存在:" << video["path"];
           }

           result.append(video);
       }

       if (qres) sqlite3_free_table(qres);
       return result;
}

int VideoPlaybackWidget::getTotalVideoCount(const QDate &date)
{
       char **qres = nullptr;
       int row = 0, col = 0;
       QString dateStr = date.toString("yyyy-MM-dd");

       QString sql = QString(
           "SELECT COUNT(*) FROM video_file WHERE DATE(record_time) = '%1'"
       ).arg(dateStr);

       qDebug() << "执行计数SQL:" << sql;

       int res = SingletonDB::getInstance()->selectSql(sql, qres, row, col);
       if (res != SQLITE_OK || row <= 0 || col <= 0)
       {
           qDebug() << "查询视频总数失败，错误码:" << res;
           if (qres) sqlite3_free_table(qres);
           return 0;
       }

       int count = qres[1] ? atoi(qres[1]) : 0; // qres[1] 是第一个数据单元格
       qDebug() << "视频总数:" << count;

       sqlite3_free_table(qres);
       return count;
}

void VideoPlaybackWidget::onPlayPauseClicked()
{
    if (currentVideoIndex < 0)
    {
        QMessageBox::warning(this, "提示", "请先选择视频");
        return;
    }

    if (playThread == nullptr)
    {
        // 没有播放线程，开始播放
        playVideo(currentVideoPath);
        playPauseBtn->setText("暂停");
        isPlaying = true;
        return;
    }

    if (isPlaying)
    {
        // 暂停播放
        if (playThread->getIsPlay())
        {
            playThread->setIsPlay(false);
            playPauseBtn->setText("播放");
            isPlaying = false;
        }
    }
    else
    {
        // 继续播放
        if (playThread->isRunning())
        {
            playThread->setIsPlay(true);
            playPauseBtn->setText("暂停");
            isPlaying = true;
        }
        else
        {
            // 线程已经结束，重新创建线程播放
            stopCurrentVideo();
            playVideo(currentVideoPath);
            playPauseBtn->setText("暂停");
            isPlaying = true;
        }
    }
}

void VideoPlaybackWidget::playVideo(const QString &path)
{
       qDebug() << "准备播放视频:" << path;
       currentVideoPath = path;

       // 文件就绪检查
       if (!isVideoFileReady(path))
       {
           QString msg = "视频文件可能正在录制中\n";
           msg += "路径: " + path + "\n";
           msg += "请等待录制完成后再播放";

           QMessageBox::warning(this, "提示", msg);
           videoDisplayLabel->setText("视频文件准备中...");
           return; // 安全退出，不会崩溃
       }

       // 原有的文件检查（可以保留，但新函数已经包含了）
       QFileInfo fileInfo(path);
       if (!fileInfo.exists())
       {
           qDebug() << "视频文件不存在:" << path;
           QMessageBox::warning(this, "错误", "视频文件不存在:\n" + path);
           videoDisplayLabel->setText("视频文件不存在");
           markVideoAsInvalid(path);
           return;
       }

    // 检查文件格式
    QString suffix = fileInfo.suffix().toLower();
    QStringList videoFormats = {"h264", "mp4", "avi", "mkv", "mov", "flv", "wmv"};

    if (!videoFormats.contains(suffix))
    {
        qDebug() << "不支持的文件格式:" << suffix << "文件路径:" << path;
        QMessageBox::warning(this, "错误",
                           QString("不支持的文件格式: %1\n请选择视频文件").arg(suffix));
        videoDisplayLabel->setText("不支持的文件格式: " + suffix);
        return;
    }

    // 先停止当前播放的线程（确保完全停止）
    stopCurrentVideo();

    // 创建新的播放线程
    playThread = new PlayThread(path);

    if (!playThread)
    {
        qDebug() << "播放线程创建失败";
        videoDisplayLabel->setText("播放线程创建失败");
        return;
    }

    // 连接信号槽
    connect(playThread, &PlayThread::sendimg, this, &VideoPlaybackWidget::updateVideoFrame, Qt::QueuedConnection);
    connect(playThread, &PlayThread::sendPosition, this, &VideoPlaybackWidget::updatePlayProgress, Qt::QueuedConnection);
    connect(playThread, &PlayThread::sendFrameInfo, this, &VideoPlaybackWidget::onFrameInfoReceived, Qt::QueuedConnection);
    connect(playThread, &PlayThread::finished, this, &VideoPlaybackWidget::onVideoFinished, Qt::QueuedConnection);  // 确保连接了finished信号

    // 初始化进度条
    progressSlider->setRange(0, 100);
    progressSlider->setValue(0);

    // 显示加载提示
    videoDisplayLabel->setText("正在加载视频...");

    // 启动线程
    playThread->start();
    isPlaying = true;
    playPauseBtn->setText("暂停");
    qDebug() << "播放线程已启动";
}

void VideoPlaybackWidget::onFrameInfoReceived(int currentFrame, int totalFrames, double fps)
{
    if (fps <= 0 || totalFrames <= 0) return;

    // 动态计算总时长和当前时长
    qint64 currentMs = 0;
    qint64 totalMs = 0;

    // 计算每帧的时长（毫秒）
    double msPerFrame = 1000.0 / fps;

    // 计算当前时间
    currentMs = (qint64)(currentFrame * msPerFrame);

    // 计算总时长
    totalMs = (qint64)(totalFrames * msPerFrame);

    // 更新进度条
    if (totalMs > 0) {
        // 设置进度条的最大值（总时长）
        if (progressSlider->maximum() != totalMs) {
            progressSlider->setMaximum(totalMs);
        }

        // 用户按住滑块时不覆盖位置
        if (!progressSlider->isSliderDown() && currentMs >= 0 && currentMs <= totalMs) {
            progressSlider->setValue(currentMs);
        }

        // 调试信息
        static int lastLogFrame = 0;
        if (currentFrame % 10 == 0 && currentFrame != lastLogFrame) {
            qDebug() << "帧信息 - 当前帧:" << currentFrame
                     << "/" << totalFrames
                     << " 当前时间:" << currentMs << "ms"
                     << " 总时间:" << totalMs << "ms"
                     << " FPS:" << fps;
            lastLogFrame = currentFrame;
        }
    }
}

void VideoPlaybackWidget::onProgressChanged(int value)
{
    if (!playThread) return;

    //按时间跳转（毫秒）
    playThread->seekTo(value);

}

void VideoPlaybackWidget::stopCurrentVideo()
{
    if (playThread)
    {
        // 先断开所有信号连接，防止析构期间信号触发
        disconnect(playThread, nullptr, this, nullptr);

        // 设置停止标志，通知工作线程退出
        playThread->setIsStop(true);
        playThread->setIsPlay(false);

        // 等待线程正常退出（给充足时间让FFmpeg自然结束循环）
        if (playThread->isRunning())
        {
            if (!playThread->wait(5000))
            {
                qWarning() << "PlayThread 未能正常退出，强制终止";
                playThread->terminate();
                playThread->wait(1000);
            }
        }

        // 线程完全退出后安全删除
        playThread->deleteLater();
        playThread = nullptr;
    }

    // 重置UI状态
    isPlaying = false;
    playPauseBtn->setText("播放");
    progressSlider->setValue(0);

    videoDisplayLabel->clear();
    videoDisplayLabel->setText("请选择视频");
}

void VideoPlaybackWidget::updateVideoFrame(QImage img)
{
    if (!img.isNull())
    {
        videoDisplayLabel->setPixmap(QPixmap::fromImage(img.scaled(
            videoDisplayLabel->width(), videoDisplayLabel->height(),
            Qt::KeepAspectRatio, Qt::SmoothTransformation
        )));
    }
}

void VideoPlaybackWidget::onSpeedChanged(int index)
{
    if (!playThread) return;

    double speed = 1.0;
    switch (index)
    {
        case 0: speed = 0.5; break;
        case 1: speed = 1.0; break;
        case 2: speed = 1.5; break;
        case 3: speed = 2.0; break;
        default: break;
    }
    playThread->setSpeedFactor(speed);
}

void VideoPlaybackWidget::onExportClicked()
{
    if (currentVideoPath.isEmpty())
    {
        QMessageBox::warning(this, "提示", "请先选择要导出的视频");
        return;
    }

    QString selectedFormat = formatCombo->currentText();
    QString suffix, filter;
    if (selectedFormat == "MP4") {
        suffix = "mp4";
        filter = "MP4文件 (*.mp4)";
    } else if (selectedFormat == "AVI") {
        suffix = "avi";
        filter = "AVI文件 (*.avi)";
    } else if (selectedFormat == "MKV") {
        suffix = "mkv";
        filter = "MKV文件 (*.mkv)";
    } else if (selectedFormat == "FLV") {
        suffix = "flv";
        filter = "FLV文件 (*.flv)";
    } else {
        suffix = "mp4";
        filter = "MP4文件 (*.mp4)";
    }

    QString defaultName = QFileInfo(currentVideoPath).baseName() + "." + suffix;
    QString savePath = QFileDialog::getSaveFileName(
        this, "导出视频", defaultName, filter);

    if (savePath.isEmpty()) return;

    // 禁用导出按钮，防止重复点击
    exportBtn->setEnabled(false);
    exportBtn->setText("导出中...");

    // 使用后台线程执行转码，避免阻塞UI
    QThread *workerThread = new QThread(this);
    TranceCodeWorker *worker = new TranceCodeWorker(currentVideoPath, savePath, selectedFormat);
    worker->moveToThread(workerThread);

    connect(workerThread, &QThread::started, worker, &TranceCodeWorker::process);
    connect(worker, &TranceCodeWorker::finished, this, [this, selectedFormat, savePath](bool success, const QString &) {
        exportBtn->setEnabled(true);
        exportBtn->setText("导出");
        if (success) {
            QMessageBox::information(this, "成功", selectedFormat + "视频导出完成: " + savePath);
        } else {
            QMessageBox::warning(this, "失败", selectedFormat + "视频导出失败");
        }
        loadVideoList(); // 刷新列表
    });
    connect(worker, &TranceCodeWorker::errorOccurred, this, [this](const QString &msg) {
        QMessageBox::warning(this, "失败", msg);
        exportBtn->setEnabled(true);
        exportBtn->setText("导出");
    });
    // 线程和worker完成后自动清理
    connect(worker, &TranceCodeWorker::finished, worker, &QObject::deleteLater);
    connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);

    workerThread->start();
}

void VideoPlaybackWidget::updatePlayProgress(qint64 pos)
{
    // 用户按住滑块时不覆盖，释放后正常更新
    if (!progressSlider->isSliderDown()) {
        progressSlider->setValue(pos);
    }
}

void VideoPlaybackWidget::onVideoFinished()
{
    qDebug() << "当前视频播放完毕，索引:" << currentVideoIndex;

    // 停止当前视频（清理资源）
    stopCurrentVideo();

    // 延迟一小段时间，确保资源释放
    QTimer::singleShot(100, this, [this]() {
        // 检查是否还有下一个视频
        if (currentVideoIndex < currentVideoList.size() - 1)
        {
            currentVideoIndex++;
            qDebug() << "自动播放下一个视频，新索引:" << currentVideoIndex;

            QString nextVideoPath = currentVideoList[currentVideoIndex]["path"];

            // 检查下一个视频文件是否存在
            QFileInfo nextFileInfo(nextVideoPath);
            if (nextFileInfo.exists() && nextFileInfo.isFile() && nextFileInfo.size() > 0)
            {
                currentVideoPath = nextVideoPath;

                // 更新列表选择
                if (currentVideoIndex < videoListWidget->count())
                {
                    videoListWidget->setCurrentRow(currentVideoIndex);
                }

                // 播放下一个视频
                playVideo(nextVideoPath);
                isPlaying = true;
                playPauseBtn->setText("暂停");
            }
            else
            {
                // 如果下一个视频无效，跳过它继续找下一个
                qDebug() << "下一个视频无效，继续查找...";
                loadNextVideo();
            }
        }
        else
        {
            // 如果是最后一个视频，显示提示
            videoDisplayLabel->setText("所有视频已播放完毕");
            isPlaying = false;
            playPauseBtn->setText("播放");
            qDebug() << "所有视频已播放完毕";
        }
    });
}

void VideoPlaybackWidget::loadNextVideo()
{
    // 查找下一个可用的视频（跳过无效文件）
       int nextIndex = currentVideoIndex + 1;
       while (nextIndex < currentVideoList.count())
       {
           QString nextPath = currentVideoList[nextIndex]["path"];
           QFileInfo fileInfo(nextPath);

           // 检查文件是否有效
           if (fileInfo.exists() && fileInfo.isFile() && fileInfo.size() > 0)
           {
               currentVideoIndex = nextIndex;
               currentVideoPath = nextPath;
               videoListWidget->setCurrentRow(currentVideoIndex);
               playVideo(currentVideoPath);
               isPlaying = true;
               playPauseBtn->setText("暂停");
               return;
           }

           // 如果当前视频无效，尝试下一个
           nextIndex++;
       }

       // 如果所有视频都播放完毕
       videoDisplayLabel->setText("所有视频已播放完毕");
       isPlaying = false;
       playPauseBtn->setText("播放");
}

void VideoPlaybackWidget::onLogoutClicked()
{
       // 1停止当前视频
       stopCurrentVideo();

       // 调用登出方法
       UserController::getInstance()->logout();

       // 发送logout信号
       emit logout();

       // 关闭回放窗口
       this->close();

       qDebug() << "用户已退出登录，回放窗口关闭";

}



void VideoPlaybackWidget::markVideoAsInvalid(const QString &path)
{
    // 可以从数据库标记该视频记录为无效
    QString sql = QString("UPDATE video_file SET status = 'invalid' WHERE storage_path = '%1'")
                 .arg(path);

    // 执行更新
    SingletonDB::getInstance()->execSql(sql);

    // 从当前列表中移除
    removeInvalidVideoFromList(path);

    // 提示用户
    qDebug() << "已标记视频为无效:" << path;
}

void VideoPlaybackWidget::removeInvalidVideoFromList(const QString &path)
{
    // 从当前视频列表中移除
    for (int i = 0; i < currentVideoList.size(); ++i)
    {
        if (currentVideoList[i]["path"] == path)
        {
            currentVideoList.removeAt(i);
            break;
        }
    }

    // 重新加载列表
    loadVideoList();
}

bool VideoPlaybackWidget::isVideoFileReady(const QString &path)
{
    QFileInfo fileInfo(path);

    // 1. 基本检查
    if (!fileInfo.exists()) {
        qDebug() << "文件不存在:" << path;
        return false;
    }

    if (!fileInfo.isFile()) {
        qDebug() << "不是文件:" << path;
        return false;
    }

    if (fileInfo.size() == 0) {
        qDebug() << "文件为空:" << path;
        return false;
    }

    // 2. 尝试打开文件（检查是否被锁定）
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "文件被锁定（可能正在写入）:" << path;
        return false;
    }
    file.close();

    // 3. 检查文件修改时间（避免刚创建的文件）
    QDateTime lastModified = fileInfo.lastModified();
    QDateTime now = QDateTime::currentDateTime();
    qint64 secondsSinceModified = lastModified.secsTo(now);

    if (secondsSinceModified < 5) { // 等待5秒确保文件写完
        qDebug() << "文件最近被修改（" << secondsSinceModified << "秒前），跳过:" << path;
        return false;
    }

    // 4. 检查文件扩展名
    QString suffix = fileInfo.suffix().toLower();
    QStringList videoFormats = {"h264", "mp4", "avi", "mkv", "mov", "flv", "wmv"};
    if (!videoFormats.contains(suffix)) {
        qDebug() << "不支持的文件格式:" << suffix;
        return false;
    }

    qDebug() << "文件检查通过:" << path
             << "大小:" << fileInfo.size()
             << "修改时间:" << secondsSinceModified << "秒前";
    return true;
}
