#include "alarminfo.h"

AlarmInfo::AlarmInfo()
: alarmId(0), thresholdValue(0), actualValue(0), status(0)
{

}

AlarmInfo::AlarmInfo(int id, const QString& type, const QString& level, double threshold,
                     double actual, const QDateTime& time, int stat)
    : alarmId(id), alarmType(type), alarmLevel(level), thresholdValue(threshold),
      actualValue(actual), alarmTime(time), status(stat)
{

}

// Getter 方法实现
int AlarmInfo::getAlarmId() const
{
    return alarmId;
}

QString AlarmInfo::getAlarmType() const
{
    return alarmType;
}

QString AlarmInfo::getAlarmLevel() const
{
    return alarmLevel;
}

double AlarmInfo::getThresholdValue() const
{
    return thresholdValue;
}

double AlarmInfo::getActualValue() const
{
    return actualValue;
}

QDateTime AlarmInfo::getAlarmTime() const
{
    return alarmTime;
}

int AlarmInfo::getStatus() const
{
    return status;
}

// Setter 方法实现
void AlarmInfo::setAlarmId(int id) {
    alarmId = id;
}

void AlarmInfo::setAlarmType(const QString& type)
{
    alarmType = type;
}

void AlarmInfo::setAlarmLevel(const QString& level)
{
    alarmLevel = level;
}

void AlarmInfo::setThresholdValue(double threshold)
{
    thresholdValue = threshold;
}

void AlarmInfo::setActualValue(double actual)
{
    actualValue = actual;
}

void AlarmInfo::setAlarmTime(const QDateTime& time)
{
    alarmTime = time;
}

void AlarmInfo::setStatus(int stat)
{
    status = stat;
}
