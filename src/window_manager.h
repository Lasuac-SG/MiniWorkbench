#pragma once

#include <QObject>
#include <QQuickWindow>
#include <QtQml/qqml.h>

/**
 * @brief WindowManager manages native window properties and dynamic resizing.
 */
class WindowManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QQuickWindow* window READ window WRITE setWindow NOTIFY windowChanged)
    QML_ELEMENT
public:
    explicit WindowManager(QObject* parent = nullptr);
    ~WindowManager() override = default;

    QQuickWindow* window() const;
    void setWindow(QQuickWindow* window);

    Q_INVOKABLE void initFramelessWindow();

signals:
    void windowChanged();

private:
    QQuickWindow* m_window = nullptr;
};