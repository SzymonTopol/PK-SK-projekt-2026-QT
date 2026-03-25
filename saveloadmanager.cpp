#include "saveloadmanager.h"

SaveLoadManager::SaveLoadManager() {}

bool SaveLoadManager::writeJsonToFile(const QString& path, const QJsonObject& json)
{
    QFile file(path);
    if (file.open(QIODevice::WriteOnly))
    {
        qDebug() << "SaveLoadManager: Błąd otwarcia pliku do zapisu!!!";
        return false;
    }

    QJsonDocument doc(json);
    file.write(doc.toJson());
    file.close();
    return true;
}

QJsonObject SaveLoadManager::readJsonFromFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "SaveLoadManager: Błąd otwarcia pliku do odczytu!!!";
        return QJsonObject();
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject())
    {
        qDebug() << "SaveLoadManager: Plik jest uszkodzony lub nie jest poprawnym JSON-em!!!";
        return QJsonObject();
    }

    return doc.object();
}
