import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import GeekDashboard 1.0

Item {
    id: root
    property var model
    property int cellWidth: 80
    property int cellHeight: 80
    property int spacing: 10

    // Switch to toggle edit mode
    property bool isEditMode: false

    TodoModel {
        id: sharedTodoModel
    }

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
                clip: true

                Loader {
                    anchors.fill: parent
                    anchors.margins: 4
                    sourceComponent: {
                        if (model.type === "heatmap") return heatmapComponent
                        if (model.type === "todo") return todoComponent
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
                id: todoComponent
                Item {
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 4

                        Text {
                            text: "Todo List"
                            color: "white"
                            font.bold: true
                            Layout.alignment: Qt.AlignHCenter
                        }

                        ListView {
                            id: todoListView
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            model: sharedTodoModel
                            clip: true
                            spacing: 2
                            delegate: Rectangle {
                                width: ListView.view.width
                                height: 30
                                color: "transparent"

                                RowLayout {
                                    anchors.fill: parent
                                    CheckBox {
                                        checked: model.isCompleted
                                        onClicked: sharedTodoModel.toggleTodo(index)
                                        contentItem: Text {
                                            text: model.title
                                            color: "white"
                                            font.strikeout: model.isCompleted
                                            verticalAlignment: Text.AlignVCenter
                                            leftPadding: 24
                                        }
                                    }
                                    Item { Layout.fillWidth: true }
                                    Button {
                                        text: "X"
                                        implicitWidth: 30
                                        implicitHeight: 30
                                        background: Rectangle {
                                            color: "transparent"
                                        }
                                        contentItem: Text {
                                            text: parent.text
                                            color: "white"
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                        onClicked: sharedTodoModel.removeTodo(index)
                                    }
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            TextField {
                                id: todoInput
                                Layout.fillWidth: true
                                placeholderText: "New task..."
                                onAccepted: {
                                    if (text.trim() !== "") {
                                        sharedTodoModel.addTodo(text)
                                        text = ""
                                    }
                                }
                            }
                            Button {
                                text: "+"
                                implicitWidth: 30
                                onClicked: {
                                    if (todoInput.text.trim() !== "") {
                                        sharedTodoModel.addTodo(todoInput.text)
                                        todoInput.text = ""
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Component {
                id: heatmapComponent
                Item {
                    GitAnalyzer {
                        id: gitAnalyzer
                        Component.onCompleted: {
                            gitAnalyzer.setRepositoryPath(".")
                            gitAnalyzer.setFileExtensions([".cpp", ".h", ".qml"])

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
                        todoModel: sharedTodoModel
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
                        let newGridX = Math.round(widgetItem.x / (root.cellWidth + root.spacing))
                        let newGridY = Math.round(widgetItem.y / (root.cellHeight + root.spacing))

                        if (newGridX < 0) newGridX = 0
                        if (newGridY < 0) newGridY = 0

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