#ifndef SAVELOADMANAGER_H
#define SAVELOADMANAGER_H

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

class SaveLoadManager
{
public:
    SaveLoadManager();

    static bool writeJsonToFile(const QString& path, const QJsonObject& json);
    static QJsonObject readJsonFromFile(const QString& path);
};

#endif // SAVELOADMANAGER_H
