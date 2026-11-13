#ifndef ALARMINFO_H
#define ALARMINFO_H
#include <QDateTime>
#include <QString>

class AlarmInfo
{
public:
    AlarmInfo();
    AlarmInfo(int id, const QString& type, const QString& level, double threshold,
              double actual, const QDateTime& time, int stat);

    int getAlarmId() const;
    QString getAlarmType() const;
    QString getAlarmLevel() const;
    double getThresholdValue() const;
    double getActualValue() const;
    QDateTime getAlarmTime() const;
    int getStatus() const;

    void setAlarmId(int id);
    void setAlarmType(const QString& type);
    void setAlarmLevel(const QString& level);
    void setThresholdValue(double threshold);
    void setActualValue(double actual);
    void setAlarmTime(const QDateTime& time);
    void setStatus(int stat);
private:
    int alarmId;
    QString alarmType;
    QString alarmLevel;
    double thresholdValue;
    double actualValue;
    QDateTime alarmTime;
    int status;

};

#endif // ALARMINFO_H
