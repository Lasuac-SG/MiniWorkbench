#include "heatmap_item.h"
#include <QPen>
#include <QBrush>
#include <algorithm>
#include <QMap>

HeatmapItem::HeatmapItem(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setOpaquePainting(false);
}

GitAnalyzer* HeatmapItem::analyzer() const
{
    return m_analyzer;
}

void HeatmapItem::setAnalyzer(GitAnalyzer* analyzer)
{
    if (m_analyzer == analyzer)
        return;

    if (m_analyzer) {
        disconnect(m_analyzer, &GitAnalyzer::analysisFinished, this, &HeatmapItem::onAnalysisFinished);
    }

    m_analyzer = analyzer;

    if (m_analyzer) {
        connect(m_analyzer, &GitAnalyzer::analysisFinished, this, &HeatmapItem::onAnalysisFinished);
    }

    emit analyzerChanged();
}

TodoModel* HeatmapItem::todoModel() const
{
    return m_todoModel;
}

void HeatmapItem::setTodoModel(TodoModel* todoModel)
{
    if (m_todoModel == todoModel)
        return;

    if (m_todoModel) {
        disconnect(m_todoModel, &TodoModel::todoStatsChanged, this, &HeatmapItem::onTodoStatsChanged);
    }

    m_todoModel = todoModel;

    if (m_todoModel) {
        connect(m_todoModel, &TodoModel::todoStatsChanged, this, &HeatmapItem::onTodoStatsChanged);
    }

    emit todoModelChanged();
    updateCombinedData();
}

void HeatmapItem::onAnalysisFinished(const QVector<DailyContribution>& contributions)
{
    m_gitContributions = contributions;
    updateCombinedData();
}

void HeatmapItem::onTodoStatsChanged()
{
    updateCombinedData();
}

void HeatmapItem::updateCombinedData()
{
    m_combinedContributions = m_gitContributions;

    if (m_todoModel) {
        QMap<QString, int> todoStats = DatabaseManager::instance().getTodoCompletionsByDate();

        // Merge into combined contributions
        for (int i = 0; i < m_combinedContributions.size(); ++i) {
            QString dateStr = m_combinedContributions[i].date;
            // E.g., dateStr format might be "yyyy-MM-dd" depending on GitAnalyzer, let's assume it matches the DB date('now')
            // GitAnalyzer generates "yyyy-MM-dd". SQLite date('now') also generates "yyyy-MM-dd".
            int todoCount = todoStats.value(dateStr, 0);

            // We use 'commits' space or additions to weight it.
            // Let's just add it to commits directly as equivalent contribution.
            // A todo completion is worth e.g. 1 commit.
            m_combinedContributions[i].commits += todoCount;
        }
    }

    m_maxScore = 0;
    for (const auto& cont : m_combinedContributions) {
        int score = cont.commits + cont.additions / 100;
        if (score > m_maxScore) {
            m_maxScore = score;
        }
    }

    update();
}

void HeatmapItem::reload()
{
    if (m_analyzer) {
        m_analyzer->startAnalysis();
    }
}

QColor HeatmapItem::getColorForScore(int score) const
{
    if (score == 0) return QColor(22, 27, 34);

    if (m_maxScore == 0) return QColor(22, 27, 34);

    float ratio = static_cast<float>(score) / static_cast<float>(m_maxScore);

    if (ratio < 0.25f) return QColor(14, 68, 41);
    if (ratio < 0.50f) return QColor(0, 109, 50);
    if (ratio < 0.75f) return QColor(38, 166, 65);
    return QColor(57, 211, 83);
}

void HeatmapItem::paint(QPainter* painter)
{
    if (m_combinedContributions.isEmpty()) {
        painter->setPen(QPen(QColor(100, 100, 100)));
        painter->drawText(boundingRect(), Qt::AlignCenter, "No contribution data");
        return;
    }

    int rows = 7;
    int cols = (m_combinedContributions.size() + 6) / 7;

    if (cols == 0) cols = 1;

    const qreal margin = 4.0;
    const qreal availableWidth = boundingRect().width() - 2 * margin;
    const qreal availableHeight = boundingRect().height() - 2 * margin;

    const qreal spacing = 3.0;

    qreal blockSizeW = (availableWidth - spacing * (cols - 1)) / cols;
    qreal blockSizeH = (availableHeight - spacing * (rows - 1)) / rows;

    qreal blockSize = std::min(blockSizeW, blockSizeH);

    if (blockSize <= 0) return;

    painter->setPen(Qt::NoPen);

    int dataIndex = 0;
    for (int c = 0; c < cols; ++c) {
        for (int r = 0; r < rows; ++r) {
            if (dataIndex >= m_combinedContributions.size()) {
                break;
            }

            const auto& cont = m_combinedContributions[dataIndex];
            int score = cont.commits + cont.additions / 100;

            QColor color = getColorForScore(score);
            painter->setBrush(QBrush(color));

            QRectF rect(
                margin + c * (blockSize + spacing),
                margin + r * (blockSize + spacing),
                blockSize,
                blockSize
            );

            painter->drawRoundedRect(rect, 2.0, 2.0);

            dataIndex++;
        }
    }
}