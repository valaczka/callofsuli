import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QZXing 3.3
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
    id: control

    title: cosClient.serverName
    icon: CosStyle.iconComputerData

    property string serverFunc: "connect"
    property var serverQueries: ({})
    property string displayText: ""
    property var extraServerInfo: ({})

    property var _infoMap: cosClient.connectionInfoMap()
    property string _url: cosClient.connectionInfo(serverFunc, serverQueries)

    maximumWidth: -1

    QTabHeader {
        id: hdr
        tabContainer: control
    }


    Grid {
        id: grid
        anchors.centerIn: parent

        columns: control.panelWidth > control.panelHeight ? gridModel.count : 1

        rowSpacing: 40
        columnSpacing: 80

        Repeater {
            model: ListModel {
                id: gridModel
            }

            ColumnLayout {
                /*anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: hdr.bottom
                anchors.bottom: parent.bottom */

                spacing: 20

                width: grid.columns > 1 ?
                           (control.panelWidth-Math.max(40, mainWindow.safeMarginLeft+mainWindow.safeMarginRight)
                            -grid.columnSpacing*(gridModel.count-1))/grid.columns :
                           (control.panelWidth-Math.max(40, mainWindow.safeMarginLeft+mainWindow.safeMarginRight))

                height: grid.columns > 1 ?
                            (control.panelHeight-Math.max(hdr.height*2, mainWindow.safeMarginTop+mainWindow.safeMarginBottom,
                                                          control.compact ? 0 : 60)) :
                            (control.panelHeight-Math.max(hdr.height*2, mainWindow.safeMarginTop+mainWindow.safeMarginBottom,
                                                          control.compact ? 0 : 60)
                             -grid.rowSpacing*(gridModel.count-1))/gridModel.count



                QLabel {
                    id: labelTop
                    visible: topText.length
                    horizontalAlignment: Label.AlignHCenter

                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                    textFormat: Text.StyledText

                    text: topText

                    color: CosStyle.colorPrimaryLighter
                    font.pixelSize: CosStyle.pixelSize*1.3

                    //width: Math.min(implicitWidth, parent.width)
                    wrapMode: Text.Wrap

                    Layout.maximumWidth: parent.width

                }

                Image {
                    id: imageQR
                    //anchors.horizontalCenter: parent.horizontalCenter

                    source: imageUrl.length ? "image://qrcode/"+Qt.btoa(imageUrl) : ""

                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Layout.minimumHeight: 100
                    Layout.minimumWidth: 100

                    fillMode: Image.PreserveAspectFit

                    sourceSize.width: 500
                    sourceSize.height: 500


                    QMenu {
                        id: menuQR

                        MenuItem {
                            icon.source: "qrc:/internal/icon/content-save.svg"
                            text: "Kép mentése"
                            onTriggered: {
                                var d = JS.dialogCreateQml("File", {
                                                               isSave: true,
                                                               folder: cosClient.getSetting("imageFolder", "")
                                                           })

                                d.accepted.connect(function(data){
                                    cosClient.saveQrImage(Qt.btoa(imageUrl), data)
                                    cosClient.setSetting("imageFolder", d.item.modelFolder)
                                })

                                d.open()

                            }
                        }

                    }

                    MouseArea {
                        id: areaQR
                        anchors.fill: imageQR
                        acceptedButtons: Qt.RightButton
                        onClicked: menuQR.popup()
                        onPressAndHold: menuQR.popup()
                    }
                }


                QLabel {
                    id: labelBottom
                    visible: bottomText.length
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    text: bottomText
                    font.pixelSize: CosStyle.pixelSize*0.9
                    font.weight: Font.Medium
                    Layout.maximumWidth: parent.width
                    wrapMode: Text.Wrap
                    horizontalAlignment: Text.AlignHCenter
                }

            }

        }
    }

    Component.onCompleted: {
        if (extraServerInfo.downloadUrl) {
            gridModel.append({
                                 topText: qsTr("Applikáció letöltése"),
                                 imageUrl: extraServerInfo.downloadUrl,
                                 bottomText: extraServerInfo.downloadUrl
                             })
        }
        gridModel.append({
                             topText: qsTr("Host: <b>%1</b><br>Port: <b>%2</b><br>SSL: <b>%3</b>")
                             .arg(_infoMap.host).arg(_infoMap.port).arg(_infoMap.ssl ? qsTr("Igen") : qsTr("Nem"))
                             +control.displayText,
                             imageUrl: _url,
                             bottomText: _url
                         })

    }


}
