#pragma once

#include <QQuickPaintedItem>
#include <QVector>
#include <QPainter>
#include <qqmlregistration.h>
#include "git_analyzer.h"
#include "todo_model.h"

class HeatmapItem : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(GitAnalyzer* analyzer READ analyzer WRITE setAnalyzer NOTIFY analyzerChanged)
    Q_PROPERTY(TodoModel* todoModel READ todoModel WRITE setTodoModel NOTIFY todoModelChanged)

public:
    explicit HeatmapItem(QQuickItem* parent = nullptr);

    void paint(QPainter* painter) override;

    GitAnalyzer* analyzer() const;
    void setAnalyzer(GitAnalyzer* analyzer);

    TodoModel* todoModel() const;
    void setTodoModel(TodoModel* todoModel);

public slots:
    void onAnalysisFinished(const QVector<DailyContribution>& contributions);
    void onTodoStatsChanged();
    void reload();

signals:
    void analyzerChanged();
    void todoModelChanged();

private:
    QVector<DailyContribution> m_gitContributions;
    QVector<DailyContribution> m_combinedContributions;
    GitAnalyzer* m_analyzer = nullptr;
    TodoModel* m_todoModel = nullptr;
    int m_maxScore = 0;

    void updateCombinedData();
    QColor getColorForScore(int score) const;
};