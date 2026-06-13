#include "widget_config_model.h"
#include "database_manager.h"

WidgetConfigModel::WidgetConfigModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_widgets = DatabaseManager::instance().loadWidgets();
    m_nextId = DatabaseManager::instance().getMaxWidgetId() + 1;

    // Add default widgets if empty
    if (m_widgets.isEmpty()) {
        addWidget("todo", 0, 0, 2, 2);
        addWidget("heatmap", 2, 0, 3, 2);
    }
}

int WidgetConfigModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_widgets.size();
}

QVariant WidgetConfigModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_widgets.size())
        return QVariant();

    const WidgetConfig &widget = m_widgets[index.row()];

    switch (role) {
    case IdRole: return widget.id;
    case TypeRole: return widget.type;
    case GridXRole: return widget.gridX;
    case GridYRole: return widget.gridY;
    case ColSpanRole: return widget.colSpan;
    case RowSpanRole: return widget.rowSpan;
    default: return QVariant();
    }
}

bool WidgetConfigModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || index.row() >= m_widgets.size())
        return false;

    WidgetConfig &widget = m_widgets[index.row()];
    bool changed = false;

    switch (role) {
    case GridXRole:
        if (widget.gridX != value.toInt()) {
            widget.gridX = value.toInt();
            changed = true;
        }
        break;
    case GridYRole:
        if (widget.gridY != value.toInt()) {
            widget.gridY = value.toInt();
            changed = true;
        }
        break;
    case ColSpanRole:
        if (widget.colSpan != value.toInt()) {
            widget.colSpan = value.toInt();
            changed = true;
        }
        break;
    case RowSpanRole:
        if (widget.rowSpan != value.toInt()) {
            widget.rowSpan = value.toInt();
            changed = true;
        }
        break;
    default:
        break;
    }

    if (changed) {
        emit dataChanged(index, index, {role});
        DatabaseManager::instance().updateWidget(widget.id, widget.gridX, widget.gridY, widget.colSpan, widget.rowSpan);
        return true;
    }
    return false;
}

QHash<int, QByteArray> WidgetConfigModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[TypeRole] = "type";
    roles[GridXRole] = "gridX";
    roles[GridYRole] = "gridY";
    roles[ColSpanRole] = "colSpan";
    roles[RowSpanRole] = "rowSpan";
    return roles;
}

void WidgetConfigModel::updateWidgetGeometry(int id, int gridX, int gridY, int colSpan, int rowSpan) {
    for (int i = 0; i < m_widgets.size(); ++i) {
        if (m_widgets[i].id == id) {
            bool changed = false;
            if (m_widgets[i].gridX != gridX) { m_widgets[i].gridX = gridX; changed = true; }
            if (m_widgets[i].gridY != gridY) { m_widgets[i].gridY = gridY; changed = true; }
            if (m_widgets[i].colSpan != colSpan) { m_widgets[i].colSpan = colSpan; changed = true; }
            if (m_widgets[i].rowSpan != rowSpan) { m_widgets[i].rowSpan = rowSpan; changed = true; }

            if (changed) {
                QModelIndex idx = index(i, 0);
                emit dataChanged(idx, idx, {GridXRole, GridYRole, ColSpanRole, RowSpanRole});
                DatabaseManager::instance().updateWidget(id, gridX, gridY, colSpan, rowSpan);
            }
            break;
        }
    }
}

void WidgetConfigModel::addWidget(const QString &type, int gridX, int gridY, int colSpan, int rowSpan) {
    beginInsertRows(QModelIndex(), m_widgets.size(), m_widgets.size());
    WidgetConfig w;
    w.id = m_nextId++;
    w.type = type;
    w.gridX = gridX;
    w.gridY = gridY;
    w.colSpan = colSpan;
    w.rowSpan = rowSpan;
    m_widgets.append(w);
    endInsertRows();

    DatabaseManager::instance().addWidget(w.id, w.type, w.gridX, w.gridY, w.colSpan, w.rowSpan);
}

void WidgetConfigModel::removeWidget(int id) {
    for (int i = 0; i < m_widgets.size(); ++i) {
        if (m_widgets[i].id == id) {
            beginRemoveRows(QModelIndex(), i, i);
            m_widgets.removeAt(i);
            endRemoveRows();

            DatabaseManager::instance().removeWidget(id);
            break;
        }
    }
}