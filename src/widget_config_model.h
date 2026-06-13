#pragma once

#include <QAbstractListModel>
#include <QtQml/qqml.h>
#include <QVector>
#include <QString>

struct WidgetConfig {
    int id;
    QString type;
    int gridX;
    int gridY;
    int colSpan;
    int rowSpan;
};

class WidgetConfigModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        TypeRole,
        GridXRole,
        GridYRole,
        ColSpanRole,
        RowSpanRole
    };

    explicit WidgetConfigModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void updateWidgetGeometry(int id, int gridX, int gridY, int colSpan, int rowSpan);
    Q_INVOKABLE void addWidget(const QString &type, int gridX, int gridY, int colSpan, int rowSpan);
    Q_INVOKABLE void removeWidget(int id);

private:
    QVector<WidgetConfig> m_widgets;
    int m_nextId = 1;
};