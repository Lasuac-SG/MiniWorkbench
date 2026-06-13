#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QDateTime>
#include <QFutureWatcher>
#include <qqmlregistration.h>

struct DailyContribution {
    QString date;
    int commits = 0;
    int additions = 0;
    int deletions = 0;
};

// Declare Q_DECLARE_METATYPE for DailyContribution so it can be used in signals/slots
Q_DECLARE_METATYPE(DailyContribution)
Q_DECLARE_METATYPE(QVector<DailyContribution>)

class GitAnalyzer : public QObject {
    Q_OBJECT
    QML_ELEMENT
public:
    explicit GitAnalyzer(QObject* parent = nullptr);
    ~GitAnalyzer() override;

    Q_INVOKABLE void setRepositoryPath(const QString& path);
    Q_INVOKABLE void setFileExtensions(const QStringList& extensions);
    Q_INVOKABLE void setDateRange(const QDateTime& since, const QDateTime& until);

    Q_INVOKABLE void startAnalysis();

signals:
    void analysisFinished(QVector<DailyContribution> contributions);
    void analysisFailed(QString errorMessage);

private:
    QString m_repoPath;
    QStringList m_extensions;
    QDateTime m_since;
    QDateTime m_until;
    QFutureWatcher<QVector<DailyContribution>> m_watcher;

    static QVector<DailyContribution> doAnalysis(QString repoPath, QStringList extensions, QDateTime since, QDateTime until);
};