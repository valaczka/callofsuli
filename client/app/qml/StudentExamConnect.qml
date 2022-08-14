import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
    id: control

    title: qsTr("Dolgozatírás")
    icon: "qrc:/internal/img/battle.png"

    Column {
        anchors.centerIn: parent
        spacing: 10

        visible: studentMaps.examEngine == ""

        QTextField {
            id: textCode
            width: Math.min(CosStyle.pixelSize*15, control.width*0.9)
            lineVisible: true

            anchors.horizontalCenter: parent.horizontalCenter

            placeholderText: qsTr("Írd be a csatlakozási kódot")

            onAccepted: btnConnect.press()
        }

        QButton {
            id: btnConnect
            anchors.horizontalCenter: parent.horizontalCenter

            text: qsTr("Csatlakozás")
            enabled: textCode.text != ""

            onClicked: {
                studentMaps.send("examEngineConnect", {code: textCode.text})
            }
        }

        QToolButtonBig {
            anchors.horizontalCenter: parent.horizontalCenter

            text: qsTr("Beolvasás")

            icon.source: "qrc:/internal/icon/qrcode-scan.svg"
            onClicked: {
                cosClient.checkMediaPermissions()
            }
        }

        QListBusyIndicator {
            anchors.horizontalCenter: parent.horizontalCenter
            height: CosStyle.pixelSize*3
            width: CosStyle.pixelSize*3
            running: true
            visible: studentMaps.examEngine != ""
        }

        QButton {
            id: btnDownload

            text: qsTr("Letöltés")

            anchors.horizontalCenter: parent.horizontalCenter
            visible: false

            Timer {
                running: studentMaps.examEngine != ""
                interval: 5000
                repeat: false
                onTriggered: btnDownload.visible = true
            }

            onClicked: studentMaps.send("examEngineMapGet", {})
        }
    }


    Component {
        id: cmpQR
        ReadQR {
            onTagFound: {
                if (studentMaps.isValidUrl(tag)) {
                    captureEnabled = false
                    control.tabPage.stack.pop(control)
                    studentMaps.parseUrl(tag)
                }
            }
        }
    }

    Connections {
        target: cosClient

        function onMediaPermissionsGranted() {
            pushContent(cmpQR, {})
        }
    }

    onPopulated: textCode.forceActiveFocus()

}



