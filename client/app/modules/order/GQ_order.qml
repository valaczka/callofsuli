import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Item {
    id: control

    implicitHeight: labelQuestion.height+col.height+35
    implicitWidth: 700

    required property var questionData
    property bool canPostpone: false
    property int mode: GameMatch.ModeNormal

    property real buttonWidth: width-60


    signal succeed()
    signal failed()
    signal postponed()
    signal answered(var answer)

    property var _drops: []
    property bool _dragInteractive: true
    property bool _modeDesc: false



    QButton {
        id: btnPostpone
        enabled: canPostpone
        visible: canPostpone
        anchors.verticalCenter: labelQuestion.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 20
        icon.source: CosStyle.iconPostpone
        text: qsTr("Később")
        themeColors: CosStyle.buttonThemeOrange
        onClicked: postponed()
    }

    QLabel {
        id: labelQuestion

        font.family: "Special Elite"
        font.pixelSize: CosStyle.pixelSize*1.4
        wrapMode: Text.Wrap
        anchors.bottom: parent.bottom
        anchors.left: btnPostpone.visible ? btnPostpone.right : parent.left
        anchors.right: btnOk.visible ? btnOk.left : parent.right
        height: Math.max(implicitHeight, btnOk.height)
        topPadding: 25
        bottomPadding: 25
        leftPadding: 20
        rightPadding: 20

        horizontalAlignment: Text.AlignHCenter

        color: CosStyle.colorAccent

        textFormat: Text.RichText

        text: questionData.question
    }


    QButton {
        id: btnOk
        enabled: (control.mode == GameMatch.ModeExam)
        //visible: (control.mode == GameMatch.ModeExam)
        anchors.verticalCenter: labelQuestion.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 20
        icon.source: "qrc:/internal/icon/check-bold.svg"
        text: qsTr("Kész")
        themeColors: CosStyle.buttonThemeGreen
        onClicked: answer()
    }



    GameQuestionTileLayout {
        id: grid
        anchors.bottom: labelQuestion.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top

        flick.contentWidth: col.width
        flick.contentHeight: col.height

        Column {
            id: col
            width: grid.flick.width
            parent: grid.flick.contentItem
            spacing: 5

        }
    }



    Component {
        id: componentTileDrop

        Item {
            implicitWidth: labelField.implicitWidth+drop.implicitWidth
            implicitHeight: Math.max(labelField.height, drop.height)

            width: parent.width
            height: implicitHeight
            anchors.horizontalCenter: parent.horizontalCenter

            property alias text: labelField.text
            property alias drop: drop

            GameQuestionTileDrop {
                id: drop
                autoResize: false
                width: parent.width

                anchors.fill: parent

                onCurrentDragChanged: recalculate()

                QLabel {
                    id: labelField

                    visible: !drop.currentDrag

                    anchors.centerIn: parent
                    width: Math.min(parent.width, implicitWidth)
                    wrapMode: Text.Wrap

                    color: CosStyle.colorPrimaryDarkest
                    font.weight: Font.Normal

                    leftPadding: 5
                    rightPadding: 5

                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
    }

    Component {
        id: componentTileDrag

        GameQuestionTileDrag {
            id: drag
            dropFlow: grid.container.flow
            mainContainer: control
            interactive: _dragInteractive

            onClicked: {
                for (var i=0; i<_drops.length; i++) {
                    var p = _drops[i]

                    if (!p.drop.currentDrag) {
                        p.drop.dropIn(drag)
                        drag.handleDropIn()
                        break
                    }
                }
            }

        }
    }


    Component.onCompleted:  {
        if (!questionData || !questionData.list)
            return

        if (questionData.mode === "descending")
            _modeDesc = true


        for (var i=0; i<questionData.list.length; i++) {
            var p = questionData.list[i]

            var t = ""

            if (i == 0)
                t = _modeDesc ? questionData.placeholderMax : questionData.placeholderMin
            else if (i == questionData.list.length-1)
                t = _modeDesc ? questionData.placeholderMin : questionData.placeholderMax

            var o = componentTileDrop.createObject(col, {text: t})

            _drops.push(o)

            componentTileDrag.createObject(grid.container.flow, {
                                               tileData: p,
                                               text: p.text
                                           })
        }
    }





    function recalculate() {
        if (!_drops.length || btnOk.enabled)
            return

        var s = true

        for (var i=0; i<_drops.length; i++) {
            var p = _drops[i]
            if (!p.drop.currentDrag) {
                s = false
                break
            }
        }

        if (s)
            btnOk.enabled = true
    }


    function answer() {
        if (mode == GameMatch.ModeExam) {
            answered({"error": "missing implementation"})
        } else {
            btnOk.enabled = false
            _dragInteractive = false

            var success = true

            var prevNum = null

            for (var i=0; i<_drops.length; i++) {
                var p = _drops[i]

                var drag = p.drop.currentDrag

                if (drag) {
                    var data = drag.tileData

                    var correct = (prevNum === null || (_modeDesc && prevNum > data.num) || (!_modeDesc && prevNum < data.num))

                    prevNum = data.num

                    if (correct) {
                        drag.type = GameQuestionButton.Correct
                    } else {
                        drag.type = GameQuestionButton.Wrong
                        success = false
                    }
                } else {
                    p.drop.isWrong = true
                    success = false
                }

            }

            if (success)
                succeed()
            else
                failed()
        }
    }


    function keyPressed(key) {
        if (btnOk.enabled && (key === Qt.Key_Enter || key === Qt.Key_Return))
            btnOk.press()
    }

}

