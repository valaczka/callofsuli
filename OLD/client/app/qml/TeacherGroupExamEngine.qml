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
    icon: "qrc:/internal/icon/calendar-multiple.svg"

    property string code: ""
    property string mapUuid: ""

    ListModel {
        id: modelUserList
    }

    QAccordion {
        id: acc

        QTabHeader {
            tabContainer: control
            flickable: acc.flickable
        }

        QCollapsible {
            title: qsTr("Adatok")

            Column {
                QLabel {
                    id: label
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "prepared\nCODE: "+code+"\n"+mapUuid
                    width: parent.width
                    wrapMode: Text.Wrap
                }

                QCheckBox {
                    id: checkAuto
                    text: qsTr("Automatikus kijelölés")
                }

                QButton {
                    anchors.horizontalCenter: parent.horizontalCenter

                    text: "stop"

                    onClicked: teacherGroups.send(CosMessage.ClassExamEngine, "stop", {})
                }
            }
        }

        QCollapsible {
            title: qsTr("Tanulók")

            rightComponent: QToolButton {
                icon.source: CosStyle.iconRefresh
                onClicked: teacherGroups.send(CosMessage.ClassExamEngine, "getMembers", {})
            }

            QObjectListView {
                id: userList

                width: parent.width

                refreshEnabled: false
                //delegateHeight: CosStyle.twoLineHeight

                isFullscreen: control.compact

                model: SortFilterProxyModel {
                    sourceModel: modelUserList

                    sorters: [
                        StringSorter { roleName: "firstname"; sortOrder: Qt.AscendingOrder; priority: 2 },
                        StringSorter { roleName: "lastname"; sortOrder: Qt.AscendingOrder; priority: 1 }
                    ]

                    proxyRoles: [
                        SwitchRole {
                            name: "background"
                            filters: [
                                ValueFilter {
                                    roleName: "status"
                                    value: "prepared"
                                    SwitchRole.value: JS.setColorAlpha(CosStyle.colorOK, 0.4)
                                },
                                ValueFilter {
                                    roleName: "status"
                                    value: "writing"
                                    SwitchRole.value: JS.setColorAlpha(CosStyle.colorWarningDark, 0.4)
                                }
                            ]
                            defaultValue: "transparent"
                        },
                        SwitchRole {
                            name: "titlecolor"
                            filters: [
                                ValueFilter {
                                    roleName: "status"
                                    value: "prepared"
                                    SwitchRole.value: CosStyle.colorAccentLight
                                },
                                ValueFilter {
                                    roleName: "status"
                                    value: "writing"
                                    SwitchRole.value: "black"
                                }
                            ]
                            defaultValue: CosStyle.colorPrimary
                        },
                        JoinRole {
                            name: "fullname"
                            roleNames: ["firstname", "lastname"]
                            separator: " "
                        }

                    ]
                }

                modelTitleRole: "fullname"
                modelSubtitleRole: "username"
                modelTitleColorRole: "titlecolor"
                modelSubtitleColorRole: "titlecolor"
                modelBackgroundRole: "background"

                highlightCurrentItem: false

                autoSelectorChange: false
                selectorSet: true

                leftComponent: QProfileImage {
                    source: model && model.picture ? model.picture : ""
                    width: userList.delegateHeight+10
                    height: userList.delegateHeight*0.8
                }
            }
        }
    }




    Connections {
        target: teacherGroups

        function onExamEngineMessage(func, json) {
            label.text += "\n"+func

            if (func === "getMembers" && json.list !== undefined) {
                JS.listModelReplaceAddSelected(modelUserList, json.list)
                if (checkAuto)
                    JS.listModelUpdateProperty(modelUserList, "status", "prepared", "selected", true)
            } else if (func === "studentPrepared") {
                JS.listModelUpdateProperty(modelUserList, "username", json.username, "status", "prepared")
                if (checkAuto)
                    JS.listModelUpdateProperty(modelUserList, "username", json.username, "selected", true)
            }
        }
    }

    onPopulated: teacherGroups.send(CosMessage.ClassExamEngine, "getMembers", {})
}



