#include "todo_model.h"

TodoModel::TodoModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_todos = DatabaseManager::instance().loadTodos();
}

int TodoModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_todos.count();
}

QVariant TodoModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_todos.count())
        return QVariant();

    const TodoItem &item = m_todos[index.row()];

    switch (role) {
        case IdRole:
            return item.id;
        case TitleRole:
            return item.title;
        case IsCompletedRole:
            return item.isCompleted;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> TodoModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[TitleRole] = "title";
    roles[IsCompletedRole] = "isCompleted";
    return roles;
}

void TodoModel::addTodo(const QString &title)
{
    QString trimmed = title.trimmed();
    if (trimmed.isEmpty()) return;

    qint64 newId = DatabaseManager::instance().addTodo(trimmed);
    if (newId != -1) {
        beginInsertRows(QModelIndex(), m_todos.count(), m_todos.count());
        m_todos.append({newId, trimmed, false});
        endInsertRows();
        emit todoStatsChanged();
    }
}

void TodoModel::removeTodo(int index)
{
    if (index < 0 || index >= m_todos.count()) return;

    qint64 id = m_todos[index].id;
    if (DatabaseManager::instance().removeTodo(id)) {
        beginRemoveRows(QModelIndex(), index, index);
        m_todos.removeAt(index);
        endRemoveRows();
        emit todoStatsChanged();
    }
}

void TodoModel::toggleTodo(int index)
{
    if (index < 0 || index >= m_todos.count()) return;

    TodoItem &item = m_todos[index];
    bool newStatus = !item.isCompleted;

    if (DatabaseManager::instance().updateTodoStatus(item.id, newStatus)) {
        item.isCompleted = newStatus;
        QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex, {IsCompletedRole});
        emit todoStatsChanged();
    }
}