#include "database_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager() {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
}

DatabaseManager::~DatabaseManager() {
    closeDatabase();
}

bool DatabaseManager::openDatabase(const QString& path) {
    QDir().mkpath(QFileInfo(path).absolutePath());
    m_db.setDatabaseName(path);
    if (!m_db.open()) {
        qWarning() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }
    bool ok = createWidgetTable();
    ok &= createTodoTable();
    return ok;
}

void DatabaseManager::closeDatabase() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::createWidgetTable() {
    QSqlQuery query(m_db);
    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS widgets (
            id INTEGER PRIMARY KEY,
            type TEXT NOT NULL,
            gridX INTEGER NOT NULL,
            gridY INTEGER NOT NULL,
            colSpan INTEGER NOT NULL,
            rowSpan INTEGER NOT NULL
        )
    )";

    if (!query.exec(createTableQuery)) {
        qWarning() << "Failed to create widgets table:" << query.lastError().text();
        return false;
    }
    return true;
}

QVector<WidgetConfig> DatabaseManager::loadWidgets() {
    QVector<WidgetConfig> widgets;
    QSqlQuery query("SELECT id, type, gridX, gridY, colSpan, rowSpan FROM widgets", m_db);

    while (query.next()) {
        WidgetConfig config;
        config.id = query.value(0).toInt();
        config.type = query.value(1).toString();
        config.gridX = query.value(2).toInt();
        config.gridY = query.value(3).toInt();
        config.colSpan = query.value(4).toInt();
        config.rowSpan = query.value(5).toInt();
        widgets.append(config);
    }

    return widgets;
}

bool DatabaseManager::addWidget(int id, const QString& type, int gridX, int gridY, int colSpan, int rowSpan) {
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO widgets (id, type, gridX, gridY, colSpan, rowSpan) VALUES (:id, :type, :gridX, :gridY, :colSpan, :rowSpan)");
    query.bindValue(":id", id);
    query.bindValue(":type", type);
    query.bindValue(":gridX", gridX);
    query.bindValue(":gridY", gridY);
    query.bindValue(":colSpan", colSpan);
    query.bindValue(":rowSpan", rowSpan);

    if (!query.exec()) {
        qWarning() << "Failed to add widget:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::updateWidget(int id, int gridX, int gridY, int colSpan, int rowSpan) {
    QSqlQuery query(m_db);
    query.prepare("UPDATE widgets SET gridX = :gridX, gridY = :gridY, colSpan = :colSpan, rowSpan = :rowSpan WHERE id = :id");
    query.bindValue(":gridX", gridX);
    query.bindValue(":gridY", gridY);
    query.bindValue(":colSpan", colSpan);
    query.bindValue(":rowSpan", rowSpan);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Failed to update widget:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::removeWidget(int id) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM widgets WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Failed to remove widget:" << query.lastError().text();
        return false;
    }
    return true;
}

int DatabaseManager::getMaxWidgetId() {
    QSqlQuery query("SELECT MAX(id) FROM widgets", m_db);
    if (query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

bool DatabaseManager::createTodoTable() {
    QSqlQuery query(m_db);
    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS todos (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            is_completed INTEGER NOT NULL DEFAULT 0,
            completed_date TEXT
        )
    )";

    if (!query.exec(createTableQuery)) {
        qWarning() << "Failed to create todos table:" << query.lastError().text();
        return false;
    }
    return true;
}

QVector<TodoItem> DatabaseManager::loadTodos() {
    QVector<TodoItem> todos;
    QSqlQuery query("SELECT id, title, is_completed FROM todos ORDER BY id ASC", m_db);

    while (query.next()) {
        TodoItem item;
        item.id = query.value(0).toLongLong();
        item.title = query.value(1).toString();
        item.isCompleted = query.value(2).toInt() != 0;
        todos.append(item);
    }
    return todos;
}

qint64 DatabaseManager::addTodo(const QString& title) {
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO todos (title, is_completed) VALUES (:title, 0)");
    query.bindValue(":title", title);

    if (!query.exec()) {
        qWarning() << "Failed to add todo:" << query.lastError().text();
        return -1;
    }
    return query.lastInsertId().toLongLong();
}

bool DatabaseManager::removeTodo(qint64 id) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM todos WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Failed to remove todo:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::updateTodoStatus(qint64 id, bool isCompleted) {
    QSqlQuery query(m_db);
    if (isCompleted) {
        query.prepare("UPDATE todos SET is_completed = 1, completed_date = date('now', 'localtime') WHERE id = :id");
    } else {
        query.prepare("UPDATE todos SET is_completed = 0, completed_date = NULL WHERE id = :id");
    }
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Failed to update todo status:" << query.lastError().text();
        return false;
    }
    return true;
}

QMap<QString, int> DatabaseManager::getTodoCompletionsByDate() {
    QMap<QString, int> stats;
    QSqlQuery query("SELECT completed_date, COUNT(*) FROM todos WHERE is_completed = 1 AND completed_date IS NOT NULL GROUP BY completed_date", m_db);

    while (query.next()) {
        QString date = query.value(0).toString();
        int count = query.value(1).toInt();
        stats.insert(date, count);
    }
    return stats;
}