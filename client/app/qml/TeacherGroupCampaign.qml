import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
    id: control

    title: qsTr("Hadjáratok")
    icon: "qrc:/internal/icon/calendar-multiple.svg"

    menu: QMenu {
        MenuItem { action: actionCampaignAdd; text: qsTr("Létrehozás") }
        MenuSeparator { }
        MenuItem { action: actionHideFilter }
    }

    ListModel {
        id: modelCampaign
    }

    QObjectListView {
        id: list

        anchors.fill: parent

        isFullscreen: control.compact

        model: SortFilterProxyModel {
            sourceModel: modelCampaign

            filters: [
                ValueFilter {
                    roleName: "finished"
                    value: false
                    enabled: actionHideFilter.checked
                }
            ]

            sorters: [
                FilterSorter {
                    AllOf {
                        ValueFilter {
                            roleName: "started"
                            value: true
                        }
                        ValueFilter {
                            roleName: "finished"
                            value: false
                        }
                    }
                    priority: 3
                },
                RoleSorter { roleName: "finished"; sortOrder: Qt.AscendingOrder; priority: 2 },
                StringSorter { roleName: "starttime"; sortOrder: Qt.DescendingOrder; priority: 1 },
                StringSorter { roleName: "endtime"; priority: 0 }
            ]

            proxyRoles: [
                ExpressionRole {
                    name: "title"
                    expression: model.description === "" ? qsTr("Hadjárat #%1").arg(model.id) : model.description
                },
                SwitchRole {
                    name: "textColor"
                    filters: [
                        AllOf {
                            ValueFilter {
                                roleName: "started"
                                value: true
                            }
                            ValueFilter {
                                roleName: "finished"
                                value: false
                            }
                            SwitchRole.value: CosStyle.colorOK
                        },
                        ValueFilter {
                            roleName: "finished"
                            value: true
                            SwitchRole.value: CosStyle.colorPrimaryDark
                        }
                    ]
                    defaultValue: CosStyle.colorPrimaryLighter
                },
                SwitchRole {
                    name: "icon"
                    filters: [
                        AllOf {
                            ValueFilter {
                                roleName: "started"
                                value: true
                            }
                            ValueFilter {
                                roleName: "finished"
                                value: false
                            }
                            SwitchRole.value: "qrc:/internal/icon/calendar-account.svg"
                        },
                        ValueFilter {
                            roleName: "finished"
                            value: true
                            SwitchRole.value: "qrc:/internal/icon/calendar-check.svg"
                        }
                    ]
                    defaultValue: "qrc:/internal/icon/calendar-clock.svg"
                },
                SwitchRole {
                    name: "fontWeight"
                    filters: AllOf {
                        ValueFilter {
                            roleName: "started"
                            value: true
                        }
                        ValueFilter {
                            roleName: "finished"
                            value: false
                        }
                        SwitchRole.value: Font.DemiBold
                    }
                    defaultValue: Font.Medium
                }
            ]
        }

        modelTitleRole: "title"
        modelSubtitleRole: "subtitle"
        modelTitleColorRole: "textColor"
        modelSubtitleColorRole: "textColor"
        modelTitleWeightRole: "fontWeight"

        autoSelectorChange: false

        refreshEnabled: true

        delegateHeight: CosStyle.twoLineHeight

        header: QTabHeader {
            tabContainer: control
            flickable: list
        }

        leftComponent: QFontImage {
            width: list.delegateHeight*1.2
            height: list.delegateHeight
            size: list.delegateHeight*0.7

            icon: model ? model.icon : ""

            color: model ? model.textColor : CosStyle.colorPrimaryLighter
        }

        /*rightComponent: QFontImage {
            width: visible ? list.delegateHeight : 0
            height: width*0.8
            size: Math.min(height*0.8, 32)

            icon: "qrc:/internal/icon/briefcase-download.svg"

            visible: model && !model.downloaded

            color: CosStyle.colorAccent
        }*/


        footer: QToolButtonFooter {
            width: list.width
            action: actionCampaignAdd
        }


        onRefreshRequest: if (teacherGroups.selectedGroupId > -1)
                              teacherGroups.send("campaignListGet", {groupid: teacherGroups.selectedGroupId})

        onClicked: {
            var o = list.model.get(index)

            control.tabPage.pushContent(cmpDetails, {
                                            campaignId: o.id,
                                            contentTitle: qsTr("%1 | %2").arg(o.title).arg(teacherGroups.selectedGroupFullName)
                                        })

        }

        onRightClicked: {
            var o = list.model.get(index)
            contextMenu.currentCampaignId = o.id
            contextMenu.popup()
        }

        onLongPressed: {
            var o = list.model.get(index)
            contextMenu.currentCampaignId = o.id
            contextMenu.popup()
        }

        QMenu {
            id: contextMenu

            property int currentCampaignId: -1

            MenuItem {
                text: qsTr("Kettőzés")
                icon.source: "qrc:/internal/icon/calendar-blank-multiple.svg"
                enabled: contextMenu.currentCampaignId != -1

                onTriggered: {
                    control.tabPage.pushContent(cmpDetails, {
                                                    campaignId: contextMenu.currentCampaignId,
                                                    contentTitle: qsTr("Új hadjárat | %1").arg(teacherGroups.selectedGroupFullName),
                                                    copyToGroup: teacherGroups.selectedGroupId
                                                })


                }
            }

            QMenu {
                id: submenu
                title: qsTr("Másolás")

                Instantiator {
                    model: ListModel {
                        id: _filteredGroupModel
                    }

                    MenuItem {
                        text: model.name+" - "+model.readableClassList
                        onClicked: {
                            control.tabPage.pushContent(cmpDetails, {
                                                            campaignId: contextMenu.currentCampaignId,
                                                            contentTitle: qsTr("Új hadjárat | %1").arg(text),
                                                            copyToGroup: model.id
                                                        })


                        }
                    }

                    onObjectAdded: submenu.insertItem(index, object)
                    onObjectRemoved: submenu.removeItem(object)
                }
            }

            MenuItem { action: actionCampaignFinish }
            MenuSeparator {}
            MenuItem { action: actionCampaignRemove }
        }

        onKeyInsertPressed: actionCampaignAdd.trigger()
        onKeyDeletePressed: actionCampaignRemove.trigger()
    }



    QIconEmpty {
        visible: !modelCampaign.count
        anchors.centerIn: parent
        textWidth: parent.width*0.75
        text: qsTr("Egyetlen hadjárat sem tartozik még ehhez a csoporthoz")
        tabContainer: control
    }


    Component {
        id: cmpDetails
        TeacherGroupCampaignDetails {  }
    }




    Action {
        id: actionCampaignFinish
        text: qsTr("Befejez")
        icon.source: "qrc:/internal/icon/calendar-check.svg"
        enabled: list.currentIndex != -1 && list.modelObject(list.currentIndex).finished == false && list.modelObject(list.currentIndex).started == true

        onTriggered: {
            var o = list.modelObject(list.currentIndex)
            var d = JS.dialogCreateQml("YesNo", {
                                           text: qsTr("Biztosan befejezed a hadjáratot?\n%1").arg(o.title),
                                           image: "qrc:/internal/icon/calendar-check.svg"
                                       })
            d.accepted.connect(function() {
                teacherGroups.send("campaignFinish", {id: o.id})
            })
            d.open()
            return true
        }
    }

    Action {
        id: actionCampaignRemove
        text: qsTr("Törlés")
        icon.source: "qrc:/internal/icon/calendar-remove.svg"
        enabled: list.currentIndex != -1 && list.modelObject(list.currentIndex).started == false

        onTriggered: {
            var o = list.modelObject(list.currentIndex)
            var d = JS.dialogCreateQml("YesNo", {
                                           text: qsTr("Biztosan törlöd a hadjáratot?\n%1").arg(o.title),
                                           image: "qrc:/internal/icon/calendar-remove.svg"
                                       })
            d.accepted.connect(function() {
                teacherGroups.send("campaignRemove", {id: o.id})
            })
            d.open()
            return true
        }
    }


    Action {
        id: actionCampaignAdd
        text: qsTr("Hadjárat létrehozás")
        icon.source: "qrc:/internal/icon/calendar-plus.svg"
        enabled: teacherGroups.selectedGroupId > -1
        onTriggered: {
            control.tabPage.pushContent(cmpDetails, {
                                            campaignId: -1,
                                            contentTitle: qsTr("Új hadjárat | %1").arg(teacherGroups.selectedGroupFullName)
                                        })
        }
    }




    Action {
        id: actionHideFilter
        text: qsTr("Lezártak elrejtése")
        checkable: true
        checked: cosClient.getSettingBool("hideClosedCampaigns", false)
        onToggled: cosClient.setSetting("hideClosedCampaigns", checked)
    }


    Connections {
        target: teacherGroups

        function onCampaignRemove(jsonData, binaryData) {
            if (teacherGroups.selectedGroupId > -1)
                teacherGroups.send("campaignListGet", {groupid: teacherGroups.selectedGroupId})
        }

        function onCampaignAdd(jsonData, binaryData) {
            if (teacherGroups.selectedGroupId > -1)
                teacherGroups.send("campaignListGet", {groupid: teacherGroups.selectedGroupId})
        }

        function onCampaignModify(jsonData, binaryData) {
            if (teacherGroups.selectedGroupId > -1)
                teacherGroups.send("campaignListGet", {groupid: teacherGroups.selectedGroupId})
        }

        function onCampaignFinish(jsonData, binaryData) {
            if (teacherGroups.selectedGroupId > -1)
                teacherGroups.send("campaignListGet", {groupid: teacherGroups.selectedGroupId})
        }

        function onCampaignListGet(jsonData, binaryData) {
            modelCampaign.clear()

            for (var i=0; i<jsonData.list.length; i++) {
                var o = jsonData.list[i]
                o.subtitle = JS.readableInterval(o.starttime, o.endtime)
                modelCampaign.append(o)
            }
        }
    }



    onPopulated: {
        list.forceActiveFocus()

        if (teacherGroups.selectedGroupId > -1)
            teacherGroups.send("campaignListGet", {groupid: teacherGroups.selectedGroupId})
    }

    Component.onCompleted: {
        _filteredGroupModel.clear()

        for (var i=0; i<teacherGroups.allGroupList.length; i++) {
            var d=teacherGroups.allGroupList[i]
            if (d.id !== teacherGroups.selectedGroupId)
                _filteredGroupModel.append({id: d.id, name: d.name, readableClassList: d.readableClassList})
        }
    }

}



