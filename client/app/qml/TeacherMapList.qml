import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
    id: control

    title: qsTr("Pályák")
    icon: CosStyle.iconPackage

    signal selectMap()

    QObjectListView {
        id: list
        anchors.fill: parent

        isFullscreen: control.compact

        refreshEnabled: true
        delegateHeight: CosStyle.twoLineHeight

        autoSelectorChange: true

        header: QTabHeader {
            tabContainer: control
            flickable: list
        }

        leftComponent: QFontImage {
            width: visible ? list.delegateHeight : 0
            height: width*0.8
            size: Math.min(height*0.8, 32)

            icon: if (model && model.used > 0)
                      "qrc:/internal/icon/briefcase-account.svg"
                  else if (model && model.binded.length)
                      "qrc:/internal/icon/briefcase-eye.svg"
                  /*else if (model && model.version > 1)
                      "qrc:/internal/icon/briefcase-clock.svg"*/
                  else
                      "qrc:/internal/icon/briefcase.svg"

            visible: model

            color: model ? model.textColor : CosStyle.colorWarning
        }

        rightComponent: QFontImage {
            width: visible ? list.delegateHeight : 0
            height: width*0.8
            size: Math.min(height*0.8, 32)

            icon: "qrc:/internal/icon/briefcase-download.svg"

            visible: model && !model.downloaded

            color: CosStyle.colorAccent
        }

        model: SortFilterProxyModel {
            sourceModel: teacherMaps.modelMapList

            sorters: [
                StringSorter { roleName: "name" }
            ]

            proxyRoles: [
                ExpressionRole {
                    name: "details"
                    expression: qsTr("%1. verzió (%2), módosítva: %3") .arg(model.version) .arg(cosClient.formattedDataSize(Number(model.dataSize))) .arg(model.lastModified.toLocaleString(Qt.locale()))

                    // TODO: JS.readableTimestamp(model.timestamp))
                },
                SwitchRole {
                    name: "textColor"
                    filters: [
                        ExpressionFilter {
                            expression: model.used > 0
                            SwitchRole.value: CosStyle.colorAccent
                        },
                        ExpressionFilter {
                            expression: model.binded.length
                            SwitchRole.value: CosStyle.colorPrimaryLighter
                        }
                    ]
                    defaultValue: CosStyle.colorPrimary
                }
            ]
        }

        modelTitleRole: "name"
        modelSubtitleRole: "details"
        modelTitleColorRole: "textColor"
        modelSubtitleColorRole: "textColor"

        highlightCurrentItem: false

        onRefreshRequest: teacherMaps.send("mapListGet", { })

        footer: Column {
            spacing: 10
            topPadding: 10

            QToolButtonFooter {
                width: list.width
                action: actionUpload
                text: qsTr("Új pálya feltöltése")
            }

            QButton {
                anchors.horizontalCenter: parent.horizontalCenter
                action: actionMapEditor
                display: AbstractButton.TextBesideIcon
                themeColors: CosStyle.buttonThemeOrange
                padding: 20
            }
        }

        onClicked: {
            teacherMaps.selectedMapId = modelObject(index).uuid
            selectMap()
        }

        onRightClicked: contextMenu.popup()

        onLongPressed: {
            if (selectorSet) {
                contextMenu.popup()
                return
            }
        }



        QMenu {
            id: contextMenu

            MenuItem { action: actionDownload }
        }
    }

    QIconEmpty {
        visible: teacherMaps.modelMapList.count === 0
        anchors.centerIn: parent
        textWidth: parent.width*0.75
        text: qsTr("Egyetlen pálya sincs még feltöltve")
        tabContainer: control
    }

    Action {
        id: actionDownload
        text: qsTr("Letöltés")
        icon.source: "qrc:/internal/icon/briefcase-download.svg"
        enabled: !teacherMaps.isBusy && (list.currentIndex !== -1 || teacherMaps.modelMapList.selectedCount)
        onTriggered: {
            var o = list.modelObject(list.currentIndex)

            var more = teacherMaps.modelMapList.selectedCount

            if (more > 0)
                teacherMaps.mapDownload({ list: teacherMaps.modelMapList.getSelectedFields("uuid") })
            else
                teacherMaps.mapDownload({ uuid: o.uuid })
        }
    }

    Action {
        id: actionUpload
        text: qsTr("Feltöltés")
        icon.source: "qrc:/internal/icon/briefcase-plus.svg"
        onTriggered: {
            var d = JS.dialogCreateQml("File", {
                                           isSave: false,
                                           folder: cosClient.getSetting("mapFolder", ""),
                                           title: qsTr("Új pálya feltöltése"),
                                           icon: "qrc:/internal/icon/briefcase-plus.svg"
                                       })
            d.accepted.connect(function(data){
                teacherMaps.mapUpload(data)
                cosClient.setSetting("mapFolder", d.item.modelFolder)
            })

            d.open()
        }
    }


    Action {
        id: actionMapEditor
        text: qsTr("Pályaszerkesztő")
        icon.source: "qrc:/internal/icon/briefcase-edit.svg"
        onTriggered: {
            JS.createPage("MapEditor", { })
        }
    }
}



