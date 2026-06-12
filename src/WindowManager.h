#pragma once

#include <QObject>
#include <QWindow>
#include <QQuickWindow>

class WindowManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QWindow* window READ window WRITE setWindow NOTIFY windowChanged)

public:
    explicit WindowManager(QObject* parent = nullptr);
    ~WindowManager() override;

    QWindow* window() const;
    void setWindow(QWindow* window);

public slots:
    void initFramelessWindow();

signals:
    void windowChanged();

private:
    QWindow* m_window;
};
