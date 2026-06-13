#include "heatmap_item.h"
#include <QPen>
#include <QBrush>
#include <algorithm>

HeatmapItem::HeatmapItem(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    // Important for transparent background / no border window scenarios
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

void HeatmapItem::onAnalysisFinished(const QVector<DailyContribution>& contributions)
{
    m_contributions = contributions;

    // Calculate max score to normalize the heatmap colors
    m_maxScore = 0;
    for (const auto& cont : m_contributions) {
        int score = cont.commits + cont.additions / 100; // basic heuristic
        if (score > m_maxScore) {
            m_maxScore = score;
        }
    }

    // Force repaint
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
    if (score == 0) return QColor(22, 27, 34); // github dark theme empty color

    if (m_maxScore == 0) return QColor(22, 27, 34);

    float ratio = static_cast<float>(score) / static_cast<float>(m_maxScore);

    if (ratio < 0.25f) return QColor(14, 68, 41);
    if (ratio < 0.50f) return QColor(0, 109, 50);
    if (ratio < 0.75f) return QColor(38, 166, 65);
    return QColor(57, 211, 83);
}

void HeatmapItem::paint(QPainter* painter)
{
    if (m_contributions.isEmpty()) {
        painter->setPen(QPen(QColor(100, 100, 100)));
        painter->drawText(boundingRect(), Qt::AlignCenter, "No contribution data");
        return;
    }

    // Basic layout for the heatmap
    // 7 rows (days of week), N columns (weeks)
    int rows = 7;
    int cols = (m_contributions.size() + 6) / 7;

    if (cols == 0) cols = 1;

    // We leave some margin
    const qreal margin = 4.0;
    const qreal availableWidth = boundingRect().width() - 2 * margin;
    const qreal availableHeight = boundingRect().height() - 2 * margin;

    // Calculate block size to fit exactly
    // size * cols + spacing * (cols - 1) = availableWidth
    // we use a fixed spacing
    const qreal spacing = 3.0;

    qreal blockSizeW = (availableWidth - spacing * (cols - 1)) / cols;
    qreal blockSizeH = (availableHeight - spacing * (rows - 1)) / rows;

    // keep blocks square
    qreal blockSize = std::min(blockSizeW, blockSizeH);

    if (blockSize <= 0) return;

    painter->setPen(Qt::NoPen);

    int dataIndex = 0;
    // We assume data is sorted from oldest to newest.
    // Standard git heatmap: weeks are columns (left to right), days are rows (top to bottom).
    for (int c = 0; c < cols; ++c) {
        for (int r = 0; r < rows; ++r) {
            if (dataIndex >= m_contributions.size()) {
                break;
            }

            const auto& cont = m_contributions[dataIndex];
            int score = cont.commits + cont.additions / 100;

            QColor color = getColorForScore(score);
            painter->setBrush(QBrush(color));

            QRectF rect(
                margin + c * (blockSize + spacing),
                margin + r * (blockSize + spacing),
                blockSize,
                blockSize
            );

            // Draw a rounded rect
            painter->drawRoundedRect(rect, 2.0, 2.0);

            dataIndex++;
        }
    }
}