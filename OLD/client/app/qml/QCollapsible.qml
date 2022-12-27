import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Rectangle {
    id: control

    width: parent ? parent.width : 100

    property int contentHeight: content.childrenRect.height+10
    property alias headerHeight: headerRect.height
    property alias rightComponent: rightLoader.sourceComponent

    property bool isFullscreen: false

    height: headerRect.height

    property bool collapsed: false
    property alias title: title.text
    property alias titleColor: title.color
    property color backgroundColor: JS.setColorAlpha(CosStyle.colorPrimaryDarkest, 0.5)
    property color lineColor: CosStyle.colorPrimaryDark
    property color contentBackgroundColor: "transparent"

    property bool interactive: true

    property bool selectorSet: false
    property bool itemSelected: false

    default property alias contents: content.data

    signal rightClicked()
    signal longClicked()
    signal selectToggled(bool withShift)

    color: "transparent"

    Rectangle {
        id: headerRect
        width: parent.width
        height: Math.max(title.implicitHeight, arrow.implicitHeight, CosStyle.halfLineHeight, rightLoader.height)

        color: backgroundColor

        Rectangle {
            width: parent.width
            height: 1
            anchors.bottom: parent.bottom
            color: lineColor
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: interactive ? (Qt.LeftButton | Qt.RightButton) : Qt.RightButton
            onClicked: {
                if (mouse.button == Qt.LeftButton) {
                    if (selectorSet)
                        control.selectToggled(mouse.modifiers & Qt.ShiftModifier)
                    else
                        collapsed = !collapsed
                } else if (mouse.button == Qt.RightButton)
                    control.rightClicked()
            }

            onPressAndHold: control.longClicked()

            QFlipable {
                id: flipable
                width: parent.height
                height: parent.height

                anchors.left: parent.left
                anchors.leftMargin: control.isFullscreen ? mainWindow.safeMarginLeft: 0
                anchors.verticalCenter: parent.verticalCenter

                visible: control.selectorSet

                mouseArea.enabled: false

                frontIcon: CosStyle.iconUnchecked
                backIcon: CosStyle.iconChecked
                color: CosStyle.colorAccent
                flipped: control.itemSelected
            }


            QFontImage {
                id: arrow
                anchors.left: flipable.visible ? flipable.right : parent.left
                anchors.leftMargin: !flipable.visible && control.isFullscreen ? mainWindow.safeMarginLeft: 0
                anchors.verticalCenter: parent.verticalCenter

                size: CosStyle.pixelSize*1.4
                color: CosStyle.colorPrimaryLighter

                icon: CosStyle.iconDown

                rotation: -90
                transformOrigin: Item.Center
            }

            QLabel {
                id: title
                anchors.left: arrow.right
                anchors.verticalCenter: parent.verticalCenter
                //width: parent.width-arrow.width
                anchors.right: rightLoader.left
                font.pixelSize: CosStyle.pixelSize*0.85
                font.weight: Font.DemiBold
                font.capitalization: Font.AllUppercase
                color: CosStyle.colorAccentLighter
                elide: Text.ElideRight
            }

            Loader {
                id: rightLoader

                anchors.right: parent.right
                anchors.rightMargin: control.isFullscreen ? mainWindow.safeMarginRight : 0
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    Rectangle {
        id: bgRect
        color: contentBackgroundColor
        anchors.fill: parent
        anchors.topMargin: headerRect.height
    }

    Item {
        id: content

        anchors.top: headerRect.bottom
        anchors.topMargin: 5
        anchors.left: parent.left
        anchors.right: parent.right

        opacity: 0.0
        visible: opacity
    }

    states: [
        State {
            name: "VISIBLE"
            when: !control.collapsed

            PropertyChanges {
                target: arrow
                rotation: 0
            }

            PropertyChanges {
                target: control
                height: headerRect.height+contentHeight+1
            }

            PropertyChanges {
                target: content
                opacity: 1.0
            }
        }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "VISIBLE"
            SequentialAnimation {
                ParallelAnimation {
                    PropertyAnimation {
                        target: arrow
                        property: "rotation"
                        duration: 225
                    }
                    PropertyAnimation {
                        target: control
                        property: "height"
                        duration: 225
                        easing.type: Easing.OutQuint
                    }
                }
                PropertyAnimation {
                    target: content
                    property: "opacity"
                    duration: 175
                    easing.type: Easing.InQuad
                }
            }
        },
        Transition {
            from: "VISIBLE"
            to: "*"
            SequentialAnimation {
                PropertyAnimation {
                    target: content
                    property: "opacity"
                    duration: 125
                    easing.type: Easing.OutQuad
                }
                ParallelAnimation {
                    PropertyAnimation {
                        target: arrow
                        property: "rotation"
                        duration: 175
                    }
                    PropertyAnimation {
                        target: control
                        property: "height"
                        duration: 175
                        easing.type: Easing.InQuint
                    }
                }
            }
        }
    ]

}
