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
    return createWidgetTable();
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