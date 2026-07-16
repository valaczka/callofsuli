import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Rectangle {
    id: root

    property bool active: false

    property int dialogImplicitWidth: implicitWidth
    property int dialogImplicitHeight: implicitHeight

    property color borderColor: Qaterial.Style.iconColor()

    default property alias _contentData: realContent.data

    signal closeRequest()

    visible: false

    layer.enabled: true

    opacity: 0.0

    color: Client.Utils.colorSetAlpha(Qaterial.Colors.black, 0.4)

    implicitWidth: 850
    implicitHeight: 350

    MouseArea {
        anchors.fill: parent
        onClicked: (mouse) => {
                       root.closeRequest()
                   }
    }


    Item {
        id: panel

        width: Math.floor(Math.min(parent.width - 2 * Qaterial.Style.card.horizontalPadding, dialogImplicitWidth))
        height: Math.floor(Math.min(parent.height - 2 * Qaterial.Style.card.verticalPadding, dialogImplicitHeight))

        anchors.centerIn: parent

        MouseArea {
            anchors.fill: panel
        }


        /*DropShadow {
                        anchors.fill: panel
                        horizontalOffset: 4
                        verticalOffset: 4
                        color: Client.Utils.colorSetAlpha("black", 0.4)
                        source: border2
                }*/

        BorderImage {
            id: border2
            source: "qrc:/internal/img/border2.svg"
            visible: false

            //sourceSize.height: 141
            //sourceSize.width: 414

            anchors.fill: panel
            border.top: 10
            border.left: 5
            border.right: 80
            border.bottom: 10

            horizontalTileMode: BorderImage.Repeat
            verticalTileMode: BorderImage.Repeat
        }


        Rectangle {
            id: metalbg
            visible: false
            anchors.fill: panel
            color: Client.Utils.colorSetAlpha(Qaterial.Colors.black, 0.6)
        }

        OpacityMask {
            id: opacity1
            anchors.fill: panel
            source: metalbg
            maskSource: border2
        }


        FocusScope {
            id: realContent
            anchors.fill: parent
            visible: true

            anchors.topMargin: 5
            anchors.leftMargin: 5
            anchors.rightMargin: 5
            anchors.bottomMargin: 10
        }


        // BORDER

        BorderImage {
            id: border1
            source: "qrc:/internal/img/border1.svg"
            visible: false

            //sourceSize.height: 141
            //sourceSize.width: 414

            anchors.fill: panel
            border.top: 15
            border.left: 10
            border.right: 60
            border.bottom: 25

            horizontalTileMode: BorderImage.Repeat
            verticalTileMode: BorderImage.Repeat
        }

        ColorOverlay {
            anchors.fill: border1
            source: border1
            color: borderColor
        }
    }



    states: [
        State {
            name: "active"
            when: root.active
            PropertyChanges {
                target: root
                opacity: 1.0
            }
            PropertyChanges {
                target: root
                visible: true
            }
        }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "active"

            SequentialAnimation {
                PropertyAction {
                    target: root
                    property: "visible"
                    value: true
                }

                PropertyAnimation {
                    target: root
                    property: "opacity"
                    duration: 75
                    easing.type: Easing.OutQuad
                }
            }
        },

        Transition {
            from: "active"
            to: "*"

            SequentialAnimation {
                PropertyAnimation {
                    target: root
                    property: "opacity"
                    duration: 125
                    easing.type: Easing.OutQuad
                }

                PropertyAction {
                    target: root
                    property: "visible"
                    value: false
                }
            }
        }
    ]

}
