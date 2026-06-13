#pragma once

#include <QString>
#include <QVector>
#include <QMap>
#include <QSqlDatabase>
#include "widget_config_model.h"

struct TodoItem {
    qint64 id;
    QString title;
    bool isCompleted;
};

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

    // Todo table methods
    bool createTodoTable();
    QVector<TodoItem> loadTodos();
    qint64 addTodo(const QString& title);
    bool removeTodo(qint64 id);
    bool updateTodoStatus(qint64 id, bool isCompleted);

    // For heatmap linkage
    QMap<QString, int> getTodoCompletionsByDate();

private:
    DatabaseManager();
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QSqlDatabase m_db;
};