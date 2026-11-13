# 农业大棚环境监控与音视频存储系统

![Qt](https://img.shields.io/badge/Qt-5.x-41CD52?logo=qt&logoColor=white)
![C++](https://img.shields.io/badge/C++-11-00599C?logo=cplusplus&logoColor=white)
![FFmpeg](https://img.shields.io/badge/FFmpeg-音视频-007808?logo=ffmpeg&logoColor=white)
![SQLite](https://img.shields.io/badge/SQLite-3-003B57?logo=sqlite&logoColor=white)

一套基于 **Qt / C++** 的农业大棚环境监控系统：通过串口实时采集温湿度、光照等传感器数据，结合 FFmpeg 实现多通道摄像头视频监控与 H.264 录像存储，超阈值自动告警并联动控制设备（LED 补光灯、风扇），支持历史视频回放与用户权限管理。

## 功能特性

- **实时环境监控**：串口读取温/湿/光传感器数据，实时数值 + 曲线图展示
- **多通道视频监控**：四通道摄像头画面同屏/单通道切换，支持录像控制
- **音视频存储与回放**：摄像头采集帧编码为 H.264 落盘，支持历史录像回放
- **智能告警联动**：数据超阈值自动写入告警记录（饼图统计 + 列表展示），并联动 LED/风扇控制
- **用户管理**：登录/注册，图形验证码校验，权限分级
- **数据可视化**：Qt Charts 实现实时曲线、周温度柱状图、告警类型饼图

## 系统架构

采用 **MVC 分层架构**，UI、业务、数据、采集线程完全解耦：

```
┌─────────────────────────────────────────────┐
│  View 层 (界面)                              │
│  MyView → MainWindow / SettingWidget        │
│  LoginWidget / RegisterWidget / VideoPlayback│
├─────────────────────────────────────────────┤
│  Control 层 (业务控制)                       │
│  DeviceControl / UserController             │
├─────────────────────────────────────────────┤
│  Model 层 (数据模型)                         │
│  DeviceModel / UserModel                    │
├─────────────────────────────────────────────┤
│  Data 层 (数据实体 + 持久化)                  │
│  Device / AlarmInfo / SingletonDB (单例)     │
├─────────────────────────────────────────────┤
│  Thread 层 (后台线程)                        │
│  CameraThread(采集) / PlayThread / TranceCode│
└─────────────────────────────────────────────┘
```

### 核心数据链路

**传感器链路**：`硬件传感器 →(串口)→ 解析温/湿/光 → 更新面板/曲线 → 阈值比对 → 告警入库 + 设备联动`

**视频链路**：`CameraThread(FFmpeg 采集/解码) → RGB→QImage → 跨线程信号 emit → UI 显示；同时 → H.264 编码 → 录像文件`

## 核心技术点

| 技术点 | 说明 |
|--------|------|
| FFmpeg 音视频 | avformat 采集 → avcodec 解码 → swscale 转 RGB → QImage 上屏；编码线程独立录制 H.264 |
| 多线程模型 | 采集、转码、播放各自独立线程，通过跨线程信号槽（Qt::QueuedConnection）与 UI 通信，避免界面卡顿 |
| 串口编程 | QSerialPort 自定义帧协议解析传感器数据，粘包/断帧处理 |
| MVC 架构 | View/Control/Model/Data 四层分离，界面与业务解耦，便于扩展 |
| 数据库单例 | SingletonDB 全局唯一 SQLite 连接，统一数据访问入口 |
| Qt Charts | 实时曲线（QSplineSeries）、柱状图、饼图动态刷新 |
| QSS 美化 | login/setwin 两套样式表 + 自定义开机动画（QGraphicsScene 图元动画） |

## 目录结构

```
SmartSecurityEnvMonitoring/
├── view/        # 界面层：主窗口、登录注册、设置页、视频回放、开机动画
├── Control/     # 业务控制层：设备控制、用户控制
├── model/       # 数据模型层：设备模型、用户模型
├── data/        # 数据层：实体类 + SQLite 单例数据库
├── thread/      # 后台线程：摄像头采集、H.264 编码、播放、转码
├── images/      # 界面图片资源
├── ffmpeglib/   # FFmpeg 开发库（头文件 + lib）
├── lib/         # sqlite3 库
├── config.ini   # 运行配置（串口、视频路径、告警阈值）
└── SmartSecurityEnvMonitoring.pro
```

## 编译运行

```bash
# 环境要求：Qt 5.x + MinGW/MSVC，FFmpeg SDK（ffmpeglib/ 已内置 Windows 版）
qmake SmartSecurityEnvMonitoring.pro
mingw32-make        # 或 MSVC nmake / Qt Creator 直接打开 .pro 构建
```

运行前在 `config.ini` 中配置串口号、录像存储路径与告警阈值；无硬件时可用虚拟串口工具（如 Virtual Serial Port Driver）模拟传感器数据。

## 界面预览

> 截图待补充：主监控界面 / 视频回放 / 告警统计

---

*本项目为个人独立开发的 Qt/C++ 桌面应用，用于嵌入式与音视频方向的学习与实践。*
