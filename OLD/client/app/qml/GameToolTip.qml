import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import QtMultimedia 5.12
import COS.Client 1.0
import "Style"
import "JScript.js" as JS

Item {
    id: control

    implicitWidth: 500
    implicitHeight: 200

    anchors.fill: parent

    focus: true

    property alias text: popup.text
    property alias details: popup.details
    property alias image: popup.image

    signal finished()

    Keys.onEscapePressed: control.state = "hide"
    Keys.onBackPressed: control.state = "hide"


    QDialogMessage {
        id: popup
        opacity: 0
        anchors.fill: parent

        type: "info"
        title: qsTr("Tudtad?")
        icon: "qrc:/internal/icon/message-question.svg"

        onDlgClose: control.state = "hide"
    }

    states: [
        State {
            name: "show"
        },
        State {
            name: "hide"
        }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "show"

            SequentialAnimation {
                ParallelAnimation {
                    PropertyAnimation {
                        target: popup
                        property: "opacity"
                        from: 0.0
                        to: 1.0
                        duration: 235
                        easing.type: Easing.OutQuad
                    }

                    PropertyAnimation {
                        target: popup
                        property: "scale"
                        from: 0.0
                        to: 1.0
                        duration: 275
                        easing.type: Easing.OutBack
                        easing.overshoot: 3
                    }
                }

                ScriptAction {
                    script: popup.populated()
                }
            }
        },

        Transition {
            from: "*"
            to: "hide"
            SequentialAnimation {
                PropertyAnimation {
                    target: popup
                    properties: "opacity, scale"
                    from: 1.0
                    to: 0.0
                    duration: 125
                    easing.type: Easing.InQuad
                }

                ScriptAction {
                    script: {
                        control.finished()
                        control.destroy()
                    }
                }
            }
        }
    ]

    Component.onCompleted: {
        control.state = "show"
    }

}
