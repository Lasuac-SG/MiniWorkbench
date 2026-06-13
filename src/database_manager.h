#pragma once

#include <QString>
#include <QVector>
#include <QSqlDatabase>
#include "widget_config_model.h"

class DatabaseManager {
public:
    static DatabaseManager& instance();

    bool openDatabase(const QString& path);
    void closeDatabase();

    bool createWidgetTable();
    QVector<WidgetConfig> loadWidgets();
    bool addWidget(int id, const QString& type, int gridX, int gridY, int colSpan, int rowSpan);
    bool updateWidget(int id, int gridX, int gridY, int colSpan, int rowSpan);
    bool removeWidget(int id);

    int getMaxWidgetId();

private:
    DatabaseManager();
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QSqlDatabase m_db;
};