import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Item {
    id: control

    implicitHeight: imageButtons || img.visible ? 500 : labelQuestion.implicitHeight+grid.height+35
    implicitWidth: 700

    required property var questionData
    property bool canPostpone: false
    property int mode: GameMatch.ModeNormal


    readonly property bool imageButtons: questionData.imageAnswers === true

    signal succeed()
    signal failed()
    signal postponed()
    signal answered(var answer)

    signal buttonReveal(GameQuestionButton original)
    signal buttonPressByKey(int num)


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
        visible: (control.mode == GameMatch.ModeExam)
        anchors.verticalCenter: labelQuestion.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 20
        icon.source: "qrc:/internal/icon/check-bold.svg"
        text: qsTr("Kész")
        themeColors: CosStyle.buttonThemeGreen
        onClicked: answer()
    }


    Item {
        id: containerItem

        readonly property bool isHorizontal: control.width > control.height

        anchors.right: parent.right
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: labelQuestion.top
        anchors.topMargin: 15

        Image {
            id: img
            source: questionData.image !== undefined ? questionData.image : ""
            visible: questionData.image !== undefined

            width: (containerItem.isHorizontal ? parent.width/2 : parent.width)
            height: (containerItem.isHorizontal ? parent.height : parent.height/2)-10

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.topMargin: 5
            anchors.leftMargin: 5

            fillMode: Image.PreserveAspectFit
            cache: false
        }

        Flickable {
            id: flick

            /*width: img.visible && containerItem.isHorizontal ? parent.width/2 : parent.width
            height: Math.min(!img.visible || containerItem.isHorizontal ? parent.height : parent.height/2,
                             flick.contentHeight)
            x: img.visible && containerItem.isHorizontal ?
                   img.width+(parent.width-img.width-width)/2 :
                   (parent.width-width)/2
            y: img.visible && !containerItem.isHorizontal ?
                   img.height+(parent.height-img.height-height)/2 :
                   (parent.height-height)/2*/

            anchors.top: img.visible && !containerItem.isHorizontal ? img.bottom : parent.top
            anchors.left: img.visible && containerItem.isHorizontal ? img.right: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.topMargin: img.visible && !containerItem.isHorizontal ? 10 : 0
            anchors.leftMargin: img.visible && containerItem.isHorizontal ? 10 : 20
            anchors.rightMargin: img.visible && containerItem.isHorizontal ? 10 : 20

            clip: true

            contentWidth: grid.width
            contentHeight: grid.height

            boundsBehavior: Flickable.StopAtBounds
            flickableDirection: Flickable.VerticalFlick

            ScrollIndicator.vertical: ScrollIndicator { }



            Grid {
                id: grid

                width: flick.width

                y: Math.max((flick.height-height)/2, 0)
                rowSpacing: 3
                columnSpacing: 3

                columns: imageButtons ? 2 : 1

                Repeater {
                    id: rptr
                    model: questionData.options

                    GameQuestionButton {
                        id: btn
                        text: modelData
                        width: imageButtons ? (grid.width-grid.columnSpacing)/2 : grid.width
                        height: imageButtons ? (flick.height-grid.rowSpacing)/2 : implicitHeight

                        image: imageButtons ? modelData : ""

                        onClicked: answer(index, btn)
                        onToggled: {
                            for (var i=0; i<rptr.model.length; i++) {
                                var b = rptr.itemAt(i)
                                if (b !== btn)
                                    b.selected = false
                            }
                        }

                        isToggle: (control.mode == GameMatch.ModeExam)


                        Connections {
                            target: control
                            function onButtonReveal(original) {
                                btn.interactive = false

                                if (original === btn && index !== questionData.answer)
                                    btn.type = GameQuestionButton.Wrong

                                if (index === questionData.answer)
                                    btn.type = GameQuestionButton.Correct
                            }

                            function onButtonPressByKey(num) {
                                if (num-1 == index && btn.interactive) {
                                    if (isToggle) {
                                        btn.selected = true
                                        btn.toggled()
                                    } else {
                                        btn.clicked()
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }



    function answer(btnIndex, btn) {
        if (mode == GameMatch.ModeExam) {
            var idx = -1
            for (var i=0; i<rptr.model.length; i++) {
                var b = rptr.itemAt(i)
                if (b.selected)
                    idx = i
            }
            answered({index: idx})
        } else {
            buttonReveal(btn)

            if (btnIndex === questionData.answer)
                succeed()
            else
                failed()
        }
    }

    function keyPressed(key) {
        if (key === Qt.Key_1 || key === Qt.Key_A)
            buttonPressByKey(1)
        else if (key === Qt.Key_2 || key === Qt.Key_B)
            buttonPressByKey(2)
        else if (key === Qt.Key_3 || key === Qt.Key_C)
            buttonPressByKey(3)
        else if (key === Qt.Key_4 || key === Qt.Key_D)
            buttonPressByKey(4)
        else if (key === Qt.Key_5 || key === Qt.Key_E)
            buttonPressByKey(5)
        else if (key === Qt.Key_6 || key === Qt.Key_F)
            buttonPressByKey(6)
    }
}

