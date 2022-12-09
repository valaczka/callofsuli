import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QTabContainer {
    id: control

    title: qsTr("Pálya részletei")
    icon: "qrc:/internal/icon/briefcase-search.svg"


    QAccordion {
        id: accordion

        visible: teacherMaps.selectedMapId != ""

        QTabHeader {
            tabContainer: control
            flickable: accordion.flickable
        }

        Column {
            id: col
            width: parent.width

            spacing: 10

            topPadding: 20
            bottomPadding: 20

            QLabel {
                id: labelName
                font.pixelSize: CosStyle.pixelSize*1.7
                font.weight: Font.Normal
                color: CosStyle.colorAccentLight

                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter

                wrapMode: Text.Wrap
                width: Math.min(implicitWidth, col.width)
            }

            QLabel {
                id: labelInfo
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: CosStyle.pixelSize
                font.weight: Font.Normal
                color: CosStyle.colorPrimaryLighter

                horizontalAlignment: Text.AlignHCenter

                wrapMode: Text.Wrap
                width: Math.min(implicitWidth, col.width)

                bottomPadding: 20

                textFormat: Text.StyledText
            }

            QButton {
                id: btnRename
                text: qsTr("Átnevezés")
                icon.source: CosStyle.iconRename
                display: AbstractButton.TextBesideIcon
                anchors.horizontalCenter: parent.horizontalCenter

                onClicked: {
                    var d = JS.dialogCreateQml("TextField", {
                                                   title: qsTr("Pálya átnevezése"),
                                                   text: qsTr("Pálya neve:"),
                                                   value: labelName.text
                                               })

                    d.accepted.connect(function(data) {
                        if (data.length)
                            teacherMaps.send("mapModify", {uuid: teacherMaps.selectedMapId, name: data})
                    })
                    d.open()
                }



            }

            QButton {
                id: btnExport
                text: qsTr("Mentés fájlba")
                icon.source: "qrc:/internal/icon/content-save.svg"
                display: AbstractButton.TextBesideIcon
                visible: !btnDownload.visible
                anchors.horizontalCenter: parent.horizontalCenter
                enabled: Qt.platform.os !== "ios"

                onClicked:  {
                    var d = JS.dialogCreateQml("File", {
                                                   isSave: true,
                                                   folder: cosClient.getSetting("mapFolder", ""),
                                                   text: labelName.text+".map"
                                               })
                    d.accepted.connect(function(data){
                        teacherMaps.mapExport(data)
                        cosClient.setSetting("mapFolder", d.item.modelFolder)
                    })

                    d.open()
                }
            }

            QButton {
                id: btnDownload
                text: qsTr("Letöltés")
                icon.source: "qrc:/internal/icon/briefcase-download.svg"
                display: AbstractButton.TextBesideIcon
                anchors.horizontalCenter: parent.horizontalCenter

                onClicked: teacherMaps.mapDownload({ uuid: teacherMaps.selectedMapId })
            }

            QButton {
                id: btnOverride
                text: qsTr("Felülírás")
                display: AbstractButton.TextBesideIcon
                anchors.horizontalCenter: parent.horizontalCenter
                icon.source: "qrc:/internal/icon/briefcase-arrow-left-right.svg"
                enabled: Qt.platform.os !== "ios"

                onClicked: {
                    var dd = JS.dialogCreateQml("YesNo", {
                                                    text: qsTr("Biztosan felül akarod írni a pályát?\n%1").arg(labelName.text),
                                                    image: "qrc:/internal/icon/briefcase-arrow-left-right.svg"
                                                })
                    dd.accepted.connect(function() {
                        var d = JS.dialogCreateQml("File", {
                                                       isSave: false,
                                                       folder: cosClient.getSetting("mapFolder", ""),
                                                       title: qsTr("Pálya felülírása"),
                                                       icon: "qrc:/internal/icon/briefcase-arrow-left-right.svg"
                                                   })
                        d.accepted.connect(function(data){
                            teacherMaps.mapOverride(data)
                            cosClient.setSetting("mapFolder", d.item.modelFolder)
                        })

                        d.open()
                    })
                    dd.open()
                }
            }

            QButton {
                id: btnDelete
                text: qsTr("Törlés")
                themeColors: CosStyle.buttonThemeRed
                icon.source: "qrc:/internal/icon/briefcase-remove.svg"
                display: AbstractButton.TextBesideIcon
                anchors.horizontalCenter: parent.horizontalCenter

                onClicked: {
                    var d = JS.dialogCreateQml("YesNo", {
                                                   text: qsTr("Biztosan törlöd a pályát a szerverről?\n%1").arg(labelName.text),
                                                   image: "qrc:/internal/icon/briefcase-remove.svg"
                                               })
                    d.accepted.connect(function() {
                        teacherMaps.send("mapRemove", {uuid: teacherMaps.selectedMapId})
                    })
                    d.open()
                }
            }


            QButton {
                id: btnExam
                text: qsTr("Dolgozatok")
                icon.source: "qrc:/internal/icon/briefcase-download.svg"
                display: AbstractButton.TextBesideIcon
                anchors.horizontalCenter: parent.horizontalCenter

                visible: !btnDownload.visible

                onClicked: control.tabPage.pushContent(cmpExamList)
            }


        }


        QCollapsible {
            id: colMissions
            title: qsTr("Küldetések")

            isFullscreen: control.compact

            Column {
                Repeater {
                    id: rptrMission
                    model: []

                    QLabel {
                        width: colMissions.width
                        elide: Text.ElideRight
                        font.weight: Font.DemiBold
                        color: CosStyle.colorAccentLight
                        text: modelData
                        leftPadding: Math.max(15, control.compact ? mainWindow.safeMarginLeft : 0)
                        rightPadding: Math.max(15, control.compact ? mainWindow.safeMarginRight : 0)
                    }
                }
            }
        }

        QCollapsible {
            id: colGroups
            title: qsTr("Hozzárendelt csoportok")

            isFullscreen: control.compact

            Column {
                Repeater {
                    id: rptrGroup
                    model: []

                    QLabel {
                        width: colMissions.width
                        elide: Text.ElideRight
                        text: "%1 (%2)".arg(modelData.name).arg(modelData.readableClassList)
                        leftPadding: Math.max(15, control.compact ? mainWindow.safeMarginLeft : 0)
                        rightPadding: Math.max(15, control.compact ? mainWindow.safeMarginRight : 0)
                    }
                }
            }
        }


    }

    QIconEmpty {
        visible: !accordion.visible
        text: qsTr("Válassz pályát")
        anchors.centerIn: parent
        textWidth: parent.width*0.75
        tabContainer: control
    }


    Component {
        id: cmpExamList
        TeacherMapExamList {

        }
    }


    Connections {
        target: teacherMaps

        function onMapDataReady(_map, _list, _ready) {
            if (!_map) {
                labelName.text = ""
                labelInfo.text = ""
                rptrGroup.model = []
                rptrMission.model = []
                btnDownload.visible = true
                return
            }

            labelName.text = _map.name
            labelInfo.text = qsTr("Verzió: <b>%1</b><br>Méret: <b>%2</b><br>Utoljára módosítva: <b>%3</b><br>Eddigi játékok száma: <b>%4</b>").arg(_map.version)
            .arg(cosClient.formattedDataSize(Number(_map.dataSize)))
            .arg(JS.readableTimestamp(_map.lastModified))
            .arg(_map.used)

            rptrGroup.model = _map.binded

            btnDownload.visible = !_map.downloaded

            if (_ready) {
                rptrMission.model = _list
            } else {
                rptrMission.model = []
            }
        }
    }

    onPopulated: teacherMaps.getSelectedMapInfo()
}

