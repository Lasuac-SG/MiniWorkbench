import QtQuick
import QtQuick.Controls
import GeekDashboard 1.0

Item {
    id: root
    property var model
    property int cellWidth: 80
    property int cellHeight: 80
    property int spacing: 10

    // Switch to toggle edit mode
    property bool isEditMode: false

    Repeater {
        model: root.model
        delegate: Rectangle {
            id: widgetItem
            color: "transparent"
            border.color: root.isEditMode ? "red" : "transparent"
            border.width: 1

            // Calculate absolute position and size based on gridX, gridY, colSpan, rowSpan
            x: model.gridX * (root.cellWidth + root.spacing)
            y: model.gridY * (root.cellHeight + root.spacing)
            width: model.colSpan * root.cellWidth + (model.colSpan - 1) * root.spacing
            height: model.rowSpan * root.cellHeight + (model.rowSpan - 1) * root.spacing

            Behavior on x { NumberAnimation { duration: 200 } }
            Behavior on y { NumberAnimation { duration: 200 } }
            Behavior on width { NumberAnimation { duration: 200 } }
            Behavior on height { NumberAnimation { duration: 200 } }

            // Inner content
            Rectangle {
                anchors.fill: parent
                anchors.margins: 2
                color: "#3388FF"
                radius: 8

                Loader {
                    anchors.fill: parent
                    anchors.margins: 4
                    sourceComponent: {
                        if (model.type === "heatmap") return heatmapComponent
                        return defaultComponent
                    }
                }
            }

            Component {
                id: defaultComponent
                Text {
                    anchors.centerIn: parent
                    text: model.type
                    color: "white"
                }
            }

            Component {
                id: heatmapComponent
                Item {
                    GitAnalyzer {
                        id: gitAnalyzer
                        Component.onCompleted: {
                            // Automatically start analysis when loaded. In a real app,
                            // repo path would come from config.
                            gitAnalyzer.setRepositoryPath(".")
                            gitAnalyzer.setFileExtensions([".cpp", ".h", ".qml"])

                            // Last 30 days
                            let until = new Date()
                            let since = new Date()
                            since.setDate(since.getDate() - 30)

                            gitAnalyzer.setDateRange(since, until)
                            gitAnalyzer.startAnalysis()
                        }
                    }

                    HeatmapItem {
                        anchors.fill: parent
                        analyzer: gitAnalyzer
                    }
                }
            }

            // Drag Handler
            DragHandler {
                id: dragHandler
                enabled: root.isEditMode
                target: widgetItem

                onActiveChanged: {
                    if (!active) {
                        // Drag ended, calculate new gridX and gridY
                        let newGridX = Math.round(widgetItem.x / (root.cellWidth + root.spacing))
                        let newGridY = Math.round(widgetItem.y / (root.cellHeight + root.spacing))

                        // Prevent negative grid coordinates
                        if (newGridX < 0) newGridX = 0
                        if (newGridY < 0) newGridY = 0

                        // Call C++ model to update geometry
                        root.model.updateWidgetGeometry(model.id, newGridX, newGridY, model.colSpan, model.rowSpan)
                    }
                }
            }

            // Resizing (right-bottom corner)
            Rectangle {
                width: 16
                height: 16
                color: "rgba(255, 255, 255, 0.5)"
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                visible: root.isEditMode

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.SizeFDiagCursor
                    property int startX
                    property int startY
                    property int startColSpan
                    property int startRowSpan

                    onPressed: (mouse) => {
                        startX = mouse.x
                        startY = mouse.y
                        startColSpan = model.colSpan
                        startRowSpan = model.rowSpan
                    }

                    onReleased: (mouse) => {
                        let dx = mouse.x - startX
                        let dy = mouse.y - startY

                        let dCols = Math.round(dx / (root.cellWidth + root.spacing))
                        let dRows = Math.round(dy / (root.cellHeight + root.spacing))

                        let newColSpan = Math.max(1, startColSpan + dCols)
                        let newRowSpan = Math.max(1, startRowSpan + dRows)

                        root.model.updateWidgetGeometry(model.id, model.gridX, model.gridY, newColSpan, newRowSpan)
                    }
                }
            }
        }
    }
}