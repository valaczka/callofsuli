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

    property ObjectListModel _dialogMapModel: teacherGroups.newMapModel(control)

    QObjectListView {
        id: list

        anchors.fill: parent

        isFullscreen: control.compact

        model: SortFilterProxyModel {
            sourceModel: teacherGroups.modelMapList
            sorters: [
                StringSorter { roleName: "name"; priority: 0 }
            ]

            proxyRoles: [
                SwitchRole {
                    name: "textColor"
                    filters: [
                        ValueFilter {
                            roleName: "active"
                            value: true
                            SwitchRole.value: CosStyle.colorOK
                        }
                    ]
                    defaultValue: CosStyle.colorPrimaryLighter
                },
                SwitchRole {
                    name: "fontWeight"
                    filters: ExpressionFilter {
                        expression: model.active
                        SwitchRole.value: Font.DemiBold
                    }
                    defaultValue: Font.Medium
                }
            ]
        }

        modelTitleRole: "name"
        modelTitleColorRole: "textColor"
        modelTitleWeightRole: "fontWeight"

        autoSelectorChange: true

        refreshEnabled: true

        delegateHeight: CosStyle.twoLineHeight

        header: QTabHeader {
            tabContainer: control
            flickable: list
        }

        leftComponent: QFlipable {
            id: flipable
            width: list.delegateHeight
            height: list.delegateHeight

            anchors.verticalCenter: parent.verticalCenter

            mouseArea.enabled: true

            frontIcon: "qrc:/internal/icon/briefcase-variant-off-outline.svg"
            backIcon: "qrc:/internal/icon/briefcase-variant.svg"
            color: model && model.active ? CosStyle.colorOKLighter : CosStyle.colorPrimaryDark
            flipped: model && model.active

            mouseArea.onClicked: {
                teacherGroups.send("groupMapActivate", {
                                       id: teacherGroups.selectedGroupId,
                                       active: !model.active,
                                       uuid: model.uuid
                                   })
            }
        }

        rightComponent: QFontImage {
            width: visible ? list.delegateHeight : 0
            height: width*0.8
            size: Math.min(height*0.8, 32)

            icon: "qrc:/internal/icon/briefcase-download.svg"

            visible: model && !model.downloaded

            color: CosStyle.colorAccent
        }


        footer: QToolButtonFooter {
            width: list.width
            action: actionMapAdd
        }


        onRefreshRequest: teacherGroups.send("groupGet", { id: teacherGroups.selectedGroupId })

        onClicked: {
            var o = list.modelObject(index)
            if (!o.downloaded)
                actionDownload.trigger()
            /*else
                control.tabPage.pushContent(componentMapView, {
                                                mapUuid: o.uuid,
                                                mapName: o.name
                                            })*/

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

            MenuItem { action: actionActivate }
            MenuItem { action: actionDeactivate }
            MenuItem { action: actionRemove }
            MenuItem { action: actionDownload }
        }


        //onKeyInsertPressed: actionMapNew.trigger()
        onKeyF2Pressed: {
            var o = teacherGroups.modelMapList.object(sourceIndex)
            if (o.active)
                actionDeactivate.trigger()
            else
                actionActivate.trigger()
        }

        onKeyDeletePressed: actionRemove.trigger()
        //onKeyF4Pressed: actionObjectiveNew.trigger()
    }



    QIconEmpty {
        visible: teacherGroups.modelMapList.count === 0
        anchors.centerIn: parent
        textWidth: parent.width*0.75
        text: qsTr("Egyetlen pálya sem tartozik még ehhez a csoporthoz")
        tabContainer: control
    }





    Connections {
        target: teacherGroups

        function onMapDownloadRequest(formattedDataSize) {
            if (teacherGroups.downloader.fullSize > cosClient.getSetting("autoDownloadBelow", 500000)) {
                var d = JS.dialogCreateQml("YesNo", {
                                               title: qsTr("Letöltés"),
                                               text: qsTr("A szerver %1 adatot akar küldeni. Elindítod a letöltést?").arg(formattedDataSize),
                                               image: "qrc:/internal/icon/briefcase-download.svg"
                                           })
                d.accepted.connect(function() {
                    var dd = JS.dialogCreateQml("Progress", { title: qsTr("Letöltés"), downloader: teacherGroups.downloader })
                    dd.closePolicy = Popup.NoAutoClose
                    dd.open()
                })

                d.open()
            } else {
                var dd = JS.dialogCreateQml("Progress", { title: qsTr("Letöltés"), downloader: teacherGroups.downloader })
                dd.closePolicy = Popup.NoAutoClose
                dd.open()
            }
        }


        function onGroupExcludedMapListGet(jsonData, binaryData) {
            if (!jsonData || jsonData.id !== teacherGroups.selectedGroupId)
                return

            if (!jsonData.list || jsonData.list.length === 0) {
                cosClient.sendMessageWarningImage("qrc:/internal/icon/briefcase-off-outline.svg", qsTr("Pálya hozzáadása"), qsTr("Nincs több hozzáadható pálya!"))
                return
            }

            _dialogMapModel.unselectAll()
            _dialogMapModel.updateJsonArray(jsonData.list, "uuid")

            var d = JS.dialogCreateQml("List", {
                                           icon: "qrc:/internal/icon/briefcase-plus.svg",
                                           title: qsTr("Pálya hozzáadása"),
                                           modelTitleRole: "name",
                                           selectorSet: true,
                                           model: _dialogMapModel
                                       })


            d.accepted.connect(function(dlgdata) {
                if (dlgdata !== true)
                    return

                var l = _dialogMapModel.getSelectedFields("uuid")
                if (l.length === 0)
                    return

                teacherGroups.send("groupMapAdd", {id: teacherGroups.selectedGroupId, list: l})
            })
            d.open()
        }
    }

    Action {
        id: actionMapAdd
        text: qsTr("Pálya hozzáadása")
        icon.source: "qrc:/internal/icon/briefcase-plus.svg"
        enabled: teacherGroups.selectedGroupId > -1
        onTriggered: {
            teacherGroups.send("groupExcludedMapListGet", {id: teacherGroups.selectedGroupId})
        }
    }

    Action {
        id: actionActivate
        text: qsTr("Aktiválás")
        icon.source: "qrc:/internal/icon/briefcase-variant.svg"
        enabled: !teacherGroups.isBusy && (list.currentIndex !== -1 || teacherGroups.modelMapList.selectedCount)
        onTriggered: {
            var o = list.modelObject(list.currentIndex)

            var more = teacherGroups.modelMapList.selectedCount

            if (more > 0)
                teacherGroups.send("groupMapActivate", {
                                       id: teacherGroups.selectedGroupId,
                                       active: true,
                                       list: teacherGroups.modelMapList.getSelectedFields("uuid")
                                   })
            else
                teacherGroups.send("groupMapActivate", {
                                       id: teacherGroups.selectedGroupId,
                                       active: true,
                                       uuid: o.uuid
                                   })
        }
    }


    Action {
        id: actionDeactivate
        text: qsTr("Inaktiválás")
        icon.source: "qrc:/internal/icon/briefcase-variant-off-outline.svg"
        enabled: !teacherGroups.isBusy && (list.currentIndex !== -1 || teacherGroups.modelMapList.selectedCount)
        onTriggered: {
            var o = list.modelObject(list.currentIndex)

            var more = teacherGroups.modelMapList.selectedCount

            if (more > 0)
                teacherGroups.send("groupMapActivate", {
                                       id: teacherGroups.selectedGroupId,
                                       active: false,
                                       list: teacherGroups.modelMapList.getSelectedFields("uuid")
                                   })
            else
                teacherGroups.send("groupMapActivate", {
                                       id: teacherGroups.selectedGroupId,
                                       active: false,
                                       uuid: o.uuid
                                   })
        }
    }

    Action {
        id: actionRemove
        text: qsTr("Eltávolítás")
        icon.source: "qrc:/internal/icon/briefcase-minus.svg"
        enabled: !teacherGroups.isBusy && (list.currentIndex !== -1 || teacherGroups.modelMapList.selectedCount)
        onTriggered: {
            var o = list.modelObject(list.currentIndex)

            var more = teacherGroups.modelMapList.selectedCount

            if (more > 0) {
                var dd = JS.dialogCreateQml("YesNo", {
                                                title: qsTr("Pálya eltávolítása"),
                                                text: qsTr("Biztosan eltávolítod a kijelölt %1 pályát?").arg(more),
                                                image: "qrc:/internal/icon/briefcase-minus.svg"
                                            })
                dd.accepted.connect(function () {
                    teacherGroups.send("groupMapRemove", {
                                           id: teacherGroups.selectedGroupId,
                                           list: teacherGroups.modelMapList.getSelectedFields("uuid") })
                })
                dd.open()
            } else {
                var d = JS.dialogCreateQml("YesNo", {
                                               title: qsTr("Pálya eltávolítása"),
                                               text: qsTr("Biztosan eltávolítod a pályát?\n%1").arg(o.name),
                                               image: "qrc:/internal/icon/briefcase-minus.svg"
                                           })
                d.accepted.connect(function () {
                    teacherGroups.send("groupMapRemove", {
                                           id: teacherGroups.selectedGroupId,
                                           uuid: o.uuid
                                       })
                })
                d.open()
            }
        }
    }



    Action {
        id: actionDownload
        text: qsTr("Letöltés")
        icon.source: "qrc:/internal/icon/briefcase-download.svg"
        enabled: !teacherGroups.isBusy && (list.currentIndex !== -1 || teacherGroups.modelMapList.selectedCount)
        onTriggered: {
            var o = list.modelObject(list.currentIndex)

            var more = teacherGroups.modelMapList.selectedCount

            if (more > 0)
                teacherGroups.mapDownload({ list: teacherGroups.modelMapList.getSelectedFields("uuid") })
            else
                teacherGroups.mapDownload({ uuid: o.uuid })
        }
    }


    onPopulated: list.forceActiveFocus()
}



