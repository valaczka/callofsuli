import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Item {
    id: toolbar

    implicitWidth: button.width * 5
    implicitHeight: Math.max(button.height, filePath.height)

    property int leftPadding: Qaterial.Style.delegate.leftPadding(Qaterial.Style.DelegateType.Icon, 1)
    property int rightPadding: Qaterial.Style.delegate.rightPadding(Qaterial.Style.DelegateType.Icon, 1)

    property int currentTagId: 0

    property var _stack: []


    QButton {
        id: button
        text: ".."
        highlighted: true
        anchors.right: parent.right
        anchors.rightMargin: parent.rightPadding
        anchors.verticalCenter: parent.verticalCenter

        onClicked: upStack()
    }

    Flow {
        id: filePath
        spacing: 5
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: parent.leftPadding
        width: parent.width-button.width-parent.leftPadding-parent.rightPadding

        Repeater {
            model: ListModel {
                id: filePathModel

                function updatePath() {
                    clear()

                    append({
                               _text: "►",
                               idx: 0
                           })

                    for (var i=0; i<_stack.length; i++) {
                        if (i>0) {
                            append({
                                       _text: "►",
                                       idx: -1
                                   })
                        }

                        append({
                                   _text: _stack[i].name,
                                   idx: i+1
                               })
                    }
                }
            }

            Qaterial.LabelCaption {
                text: _text
                color: idx > 0 ? Qaterial.Style.primaryTextColor() : Qaterial.Style.accentColor

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    enabled: idx >= 0 && idx < _stack.length
                    onClicked: {
                        _stack.splice(idx, _stack.length-idx)

                        updateStack()
                    }
                }
            }
        }
    }

    function updateStack() {
        currentTagId = _stack.length > 0 ? _stack[_stack.length-1].id : 0
        filePathModel.updatePath()
        button.enabled = _stack.length > 0
    }

    function resetStack() {
        _stack = []
        updateStack()
    }

    function addToStack(id, name) {
        _stack.push({
                        id: id,
                        name: name
                    })
        updateStack()
    }

    function upStack() {
        if (_stack.length > 0) {
            _stack.pop()
            updateStack()

            return true
        }

        return false
    }

    Component.onCompleted: resetStack()
}
