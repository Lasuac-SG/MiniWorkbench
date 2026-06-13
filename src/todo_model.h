#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <qqmlregistration.h>
#include "database_manager.h"

class TodoModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

public:
    enum TodoRoles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        IsCompletedRole
    };

    explicit TodoModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addTodo(const QString &title);
    Q_INVOKABLE void removeTodo(int index);
    Q_INVOKABLE void toggleTodo(int index);

signals:
    void todoStatsChanged(); // Fired when a todo's completion status changes

private:
    QVector<TodoItem> m_todos;
};