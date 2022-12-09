import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Item {
    id: control

    implicitWidth: 650
    implicitHeight: 500

    required property var questionData
    property bool canPostpone: false
    property int mode: GameMatch.ModeNormal

    signal succeed()
    signal failed()
    signal postponed()
    signal answered(var answer)


    property var _drops: []
    property bool _dragInteractive: true


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

        text: qsTr("Egészítsd ki a szöveget!")
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

        flick.contentHeight: wordFlow.height
        flick.contentWidth: wordFlow.width

        Flow {
            id: wordFlow
            width: grid.flick.width
            parent: grid.flick.contentItem

            spacing: 5

            Behavior on height {
                SmoothedAnimation { duration: 125 }
            }

            Behavior on width {
                SmoothedAnimation { duration: 125 }
            }

            move: Transition {
                NumberAnimation { properties: "x,y"; duration: 125; easing.type: Easing.OutQuad }
            }
        }
    }

    Component {
        id: componentWord
        QLabel {
            height: 40
            verticalAlignment: Text.AlignVCenter
            color: CosStyle.colorPrimaryLighter
            font.weight: Font.Medium
        }
    }

    Component {
        id: componentTileDrop

        GameQuestionTileDrop {
            onCurrentDragChanged: recalculate()
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

                    if (!p.item.currentDrag) {
                        p.item.dropIn(drag)
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

        for (var i=0; i<questionData.list.length; i++) {
            var p = questionData.list[i]

            if (p.w) {
                componentWord.createObject(wordFlow, { text: p.w })
            } else if (p.q) {
                var o = componentTileDrop.createObject(wordFlow)
                var d = {}
                d.item = o
                d.correct = null

                if (questionData.answer)
                    d.correct = questionData.answer[p.q]

                _drops.push(d)
            }

        }

        if (!questionData.options)
            return

        for (i=0; i<questionData.options.length; i++) {
            var t = questionData.options[i]
            componentTileDrag.createObject(grid.container.flow, {
                                               tileData: t,
                                               text: t
                                           })

        }

    }


    function recalculate() {
        if (!_drops.length || btnOk.enabled)
            return

        var s = true

        for (var i=0; i<_drops.length; i++) {
            var p = _drops[i]
            if (!p.item.currentDrag) {
                s = false
                break
            }
        }

        if (s)
            btnOk.enabled = true
    }

    function answer() {
        btnOk.enabled = false
        _dragInteractive = false


        if (mode == GameMatch.ModeExam) {
            for (var ii=0; ii<questionData.list.length; ii++) {
                var pp = questionData.list[ii]

                if (pp.w) {
                    //componentWord.createObject(wordFlow, { text: p.w })
                } else if (pp.q) {
                    /*var o = componentTileDrop.createObject(wordFlow)
                    var d = {}
                    d.item = o
                    d.correct = null*/
                }
            }


            answered({"error": "missing implementation"})
        } else {

            var success = true

            for (var i=0; i<_drops.length; i++) {
                var p = _drops[i]

                if (p.item && p.correct) {
                    var drag = p.item.currentDrag

                    if (drag) {
                        var data = drag.tileData

                        if (data === p.correct) {
                            drag.type = GameQuestionButton.Correct
                        } else {
                            drag.type = GameQuestionButton.Wrong
                            success = false
                        }
                    } else {
                        p.item.isWrong = true
                        success = false
                    }

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
