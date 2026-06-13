#include "git_analyzer.h"
#include <QtConcurrent/QtConcurrent>
#include <QMap>
#include <git2.h>
#include <memory>

// RAII wrappers for libgit2 resources
using GitRepoPtr = std::unique_ptr<git_repository, decltype(&git_repository_free)>;
using GitRevwalkPtr = std::unique_ptr<git_revwalk, decltype(&git_revwalk_free)>;
using GitCommitPtr = std::unique_ptr<git_commit, decltype(&git_commit_free)>;
using GitTreePtr = std::unique_ptr<git_tree, decltype(&git_tree_free)>;
using GitDiffPtr = std::unique_ptr<git_diff, decltype(&git_diff_free)>;

struct DiffPayload {
    const QStringList* extensions;
    int additions;
    int deletions;
};

extern "C" {
    static int line_cb(const git_diff_delta *delta, const git_diff_hunk *, const git_diff_line *line, void *payload) {
        DiffPayload* p = static_cast<DiffPayload*>(payload);

        QString path = QString::fromUtf8(delta->new_file.path);
        bool match = p->extensions->isEmpty();
        if (!match) {
            for (const QString& ext : *(p->extensions)) {
                if (path.endsWith(ext, Qt::CaseInsensitive)) {
                    match = true;
                    break;
                }
            }
        }

        if (match) {
            if (line->origin == GIT_DIFF_LINE_ADDITION) {
                p->additions++;
            } else if (line->origin == GIT_DIFF_LINE_DELETION) {
                p->deletions++;
            }
        }
        return 0;
    }
}

GitAnalyzer::GitAnalyzer(QObject* parent)
    : QObject(parent)
{
    qRegisterMetaType<QVector<DailyContribution>>("QVector<DailyContribution>");
    git_libgit2_init();

    connect(&m_watcher, &QFutureWatcher<QVector<DailyContribution>>::finished, this, [this]() {
        emit analysisFinished(m_watcher.result());
    });
}

GitAnalyzer::~GitAnalyzer() {
    if (m_watcher.isRunning()) {
        m_watcher.waitForFinished();
    }
    git_libgit2_shutdown();
}

void GitAnalyzer::setRepositoryPath(const QString& path) {
    m_repoPath = path;
}

void GitAnalyzer::setFileExtensions(const QStringList& extensions) {
    m_extensions = extensions;
}

void GitAnalyzer::setDateRange(const QDateTime& since, const QDateTime& until) {
    m_since = since;
    m_until = until;
}

void GitAnalyzer::startAnalysis() {
    if (m_watcher.isRunning()) {
        return; // Already running
    }

    QFuture<QVector<DailyContribution>> future = QtConcurrent::run(&GitAnalyzer::doAnalysis, m_repoPath, m_extensions, m_since, m_until);
    m_watcher.setFuture(future);
}

QVector<DailyContribution> GitAnalyzer::doAnalysis(QString repoPath, QStringList extensions, QDateTime since, QDateTime until) {
    QMap<QString, DailyContribution> dailyStats;

    if (repoPath.isEmpty()) {
        return {};
    }

    git_repository* raw_repo = nullptr;
    if (git_repository_open(&raw_repo, repoPath.toUtf8().constData()) != 0) {
        return {};
    }
    GitRepoPtr repo(raw_repo, git_repository_free);

    git_revwalk* raw_walker = nullptr;
    if (git_revwalk_new(&raw_walker, repo.get()) != 0) {
        return {};
    }
    GitRevwalkPtr walker(raw_walker, git_revwalk_free);

    git_revwalk_push_head(walker.get());
    git_revwalk_sorting(walker.get(), GIT_SORT_TIME);

    git_oid oid;
    while (git_revwalk_next(&oid, walker.get()) == 0) {
        git_commit* raw_commit = nullptr;
        if (git_commit_lookup(&raw_commit, repo.get(), &oid) != 0) {
            continue;
        }
        GitCommitPtr commit(raw_commit, git_commit_free);

        git_time_t time = git_commit_time(commit.get());
        QDateTime commitDate = QDateTime::fromSecsSinceEpoch(time);

        if (since.isValid() && commitDate < since) {
            continue;
        }
        if (until.isValid() && commitDate > until) {
            continue;
        }

        QString dateStr = commitDate.toString("yyyy-MM-dd");
        if (!dailyStats.contains(dateStr)) {
            dailyStats[dateStr] = DailyContribution{dateStr, 0, 0, 0};
        }
        dailyStats[dateStr].commits++;

        // Get diff to parent
        git_tree* raw_tree = nullptr;
        if (git_commit_tree(&raw_tree, commit.get()) != 0) {
            continue;
        }
        GitTreePtr tree(raw_tree, git_tree_free);

        git_tree* raw_parent_tree = nullptr;
        git_commit* raw_parent = nullptr;
        if (git_commit_parent(&raw_parent, commit.get(), 0) == 0) {
            GitCommitPtr parent(raw_parent, git_commit_free);
            git_commit_tree(&raw_parent_tree, parent.get());
        }
        GitTreePtr parent_tree(raw_parent_tree, git_tree_free);

        git_diff* raw_diff = nullptr;
        git_diff_options diff_opts = GIT_DIFF_OPTIONS_INIT;
        if (git_diff_tree_to_tree(&raw_diff, repo.get(), parent_tree.get(), tree.get(), &diff_opts) == 0) {
            GitDiffPtr diff(raw_diff, git_diff_free);

            DiffPayload payload;
            payload.extensions = &extensions;
            payload.additions = 0;
            payload.deletions = 0;

            git_diff_foreach(diff.get(), nullptr, nullptr, nullptr, line_cb, &payload);

            dailyStats[dateStr].additions += payload.additions;
            dailyStats[dateStr].deletions += payload.deletions;
        }
    }

    QVector<DailyContribution> result;
    for (const auto& stat : dailyStats) {
        result.append(stat);
    }
    return result;
}