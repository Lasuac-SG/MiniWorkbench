#pragma once

#include <QQuickPaintedItem>
#include <QVector>
#include <QPainter>
#include <qqmlregistration.h>
#include "git_analyzer.h"

class HeatmapItem : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(GitAnalyzer* analyzer READ analyzer WRITE setAnalyzer NOTIFY analyzerChanged)

public:
    explicit HeatmapItem(QQuickItem* parent = nullptr);

    void paint(QPainter* painter) override;

    GitAnalyzer* analyzer() const;
    void setAnalyzer(GitAnalyzer* analyzer);

public slots:
    void onAnalysisFinished(const QVector<DailyContribution>& contributions);
    void reload();

signals:
    void analyzerChanged();

private:
    QVector<DailyContribution> m_contributions;
    GitAnalyzer* m_analyzer = nullptr;
    int m_maxScore = 0;

    QColor getColorForScore(int score) const;
};