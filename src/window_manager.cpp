#include "window_manager.h"
#include <QDebug>

WindowManager::WindowManager(QObject* parent) : QObject(parent) {
}

QQuickWindow* WindowManager::window() const {
    return m_window;
}

void WindowManager::setWindow(QQuickWindow* window) {
    if (m_window != window) {
        m_window = window;
        emit windowChanged();
    }
}

void WindowManager::initFramelessWindow() {
    if (!m_window) return;

    m_window->setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    m_window->setColor(Qt::transparent);
}