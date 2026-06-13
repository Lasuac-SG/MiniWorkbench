import QtQuick
import QtQuick.Window
import App.Core 1.0

Window {
    id: root
    visible: true
    title: "Desktop Geek Dashboard"

    // Base dimensions
    property int expandedWidth: 400
    property int expandedHeight: 600
    property int collapsedWidth: 40
    property int collapsedHeight: 120

    // Current state
    property bool isExpanded: false
    property bool isDarkTheme: false // Light by default

    // Theme colors
    property color bgColor: isDarkTheme ? "#1E1E1E" : "#FFFFFF"
    property color textColor: isDarkTheme ? "#FFFFFF" : "#333333"

    // Set initial window geometry to collapsed state
    width: collapsedWidth
    height: collapsedHeight
    x: Screen.desktopAvailableWidth - collapsedWidth
    y: (Screen.desktopAvailableHeight - collapsedHeight) / 2

    // Ensure the background of the Window itself is fully transparent
    color: "transparent"

    WindowManager {
        id: winManager
        window: root
        Component.onCompleted: {
            winManager.initFramelessWindow()
        }
    }

    WidgetConfigModel {
        id: widgetModel
    }

    // The visual UI element that we actually see and animate
    Rectangle {
        id: visualContainer
        color: root.bgColor
        radius: isExpanded ? 16 : 20
        border.color: root.isDarkTheme ? "#444444" : "#CCCCCC"
        border.width: 1

        // Dimensions of the visual container
        width: root.collapsedWidth
        height: root.collapsedHeight

        // Position inside the window
        x: 0
        y: 0

        Behavior on width { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
        Behavior on height { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
        Behavior on x { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
        Behavior on y { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }

        HoverHandler {
            id: hoverHandler
            onHoveredChanged: {
                if (hovered) {
                    expandTimer.start()
                    collapseTimer.stop()
                } else {
                    expandTimer.stop()
                    collapseTimer.start()
                }
            }
        }

        // Inner content representation
        Text {
            anchors.centerIn: parent
            text: root.isExpanded ? "" : "..."
            color: root.textColor
            opacity: root.isExpanded ? 0.0 : (visualContainer.width == root.collapsedWidth ? 1.0 : 0.0)
            font.pixelSize: 12

            Behavior on opacity { NumberAnimation { duration: 200 } }
        }

        Item {
            id: expandedContent
            anchors.fill: parent
            anchors.margins: 20
            opacity: root.isExpanded ? 1.0 : 0.0
            visible: opacity > 0

            Behavior on opacity { NumberAnimation { duration: 300 } }

            Text {
                text: "Dashboard " + (gridLayout.isEditMode ? "[Edit Mode]" : "")
                color: root.textColor
                font.pixelSize: 20
                font.bold: true
                y: 0
            }

            // Right click area to toggle edit mode
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: (mouse) => {
                    if (mouse.button === Qt.RightButton) {
                        gridLayout.isEditMode = !gridLayout.isEditMode
                    }
                }
            }

            GridLayoutEngine {
                id: gridLayout
                y: 40
                width: parent.width
                height: parent.height - 40
                model: widgetModel
            }
        }
    }

    Timer {
        id: expandTimer
        interval: 500
        onTriggered: expandWindow()
    }

    Timer {
        id: collapseTimer
        interval: 300
        onTriggered: collapseWindow()
    }

    function expandWindow() {
        if (root.isExpanded) return;

        // 1. Instantly expand the native window
        root.x = Screen.desktopAvailableWidth - expandedWidth
        root.y = (Screen.desktopAvailableHeight - expandedHeight) / 2
        root.width = expandedWidth
        root.height = expandedHeight

        // 2. Adjust visual container to offset the window movement so it doesn't jump
        visualContainer.x = expandedWidth - collapsedWidth
        visualContainer.y = (expandedHeight - collapsedHeight) / 2

        // 3. Trigger animations to the expanded size and position
        visualContainer.x = 0
        visualContainer.y = 0
        visualContainer.width = expandedWidth
        visualContainer.height = expandedHeight

        root.isExpanded = true
    }

    function collapseWindow() {
        if (!root.isExpanded) return;

        // Note: For collapse, we FIRST animate the visual container down to capsule size,
        // and THEN shrink the native window once the animation finishes.

        root.isExpanded = false

        visualContainer.width = collapsedWidth
        visualContainer.height = collapsedHeight
        visualContainer.x = expandedWidth - collapsedWidth
        visualContainer.y = (expandedHeight - collapsedHeight) / 2

        shrinkWindowTimer.start()
    }

    Timer {
        id: shrinkWindowTimer
        interval: 300 // Match the animation duration
        onTriggered: {
            if (root.isExpanded) return; // In case we expanded again during the animation

            // Instantly shrink the native window back
            root.x = Screen.desktopAvailableWidth - collapsedWidth
            root.y = (Screen.desktopAvailableHeight - collapsedHeight) / 2
            root.width = collapsedWidth
            root.height = collapsedHeight

            // Reset visual container offsets since the window moved
            visualContainer.x = 0
            visualContainer.y = 0
        }
    }
}
