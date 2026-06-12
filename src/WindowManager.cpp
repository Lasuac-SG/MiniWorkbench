#include "WindowManager.h"
#include <QGuiApplication>

WindowManager::WindowManager(QObject* parent)
    : QObject(parent), m_window(nullptr)
{
}

WindowManager::~WindowManager() = default;

QWindow* WindowManager::window() const {
    return m_window;
}

void WindowManager::setWindow(QWindow* window) {
    if (m_window != window) {
        m_window = window;
        emit windowChanged();
    }
}

void WindowManager::initFramelessWindow() {
    if (m_window) {
        m_window->setFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
        m_window->setColor(Qt::transparent);
    }
}
