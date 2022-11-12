import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
    id: control

    title: qsTr("Résztvevők")
    icon: CosStyle.iconGroupsSmall

    menu: QMenu {
        MenuItem { action: actionUserEdit }
    }

    property ListModel modelUserList: ListModel {}

    QObjectListView {
        id: userList
        anchors.fill: parent

        isFullscreen: control.compact

        refreshEnabled: true
        delegateHeight: CosStyle.twoLineHeight

        section.property: "classname"
        section.criteria: ViewSection.FullString
        section.delegate: Component {
            Rectangle {
                width: userList.width
                height: childrenRect.height
                color: CosStyle.colorPrimaryDark

                required property string section

                QLabel {
                    text: parent.section
                    font.pixelSize: CosStyle.pixelSize*0.8
                    font.weight: Font.DemiBold
                    font.capitalization: Font.AllUppercase
                    color: "white"

                    leftPadding: 5
                    topPadding: 2
                    bottomPadding: 2
                    rightPadding: 5

                    elide: Text.ElideRight
                }
            }
        }


        header: QTabHeader {
            tabContainer: control
            flickable: userList
        }

        leftComponent: Image {
            source: model ? cosClient.rankImageSource(model.rankid, model.ranklevel, model.rankimage) : ""
            width: userList.delegateHeight+10
            height: userList.delegateHeight*0.8
            fillMode: Image.PreserveAspectFit
        }


        rightComponent: Column {
            readonly property bool _showNumbers: userList.width > 700
            QLabel {
                anchors.right: parent.right
                text: model ? "%1 XP".arg(Number(model.sumxp).toLocaleString()) : ""
                font.weight: Font.Normal
                font.pixelSize: userList.delegateHeight*0.4
                color: CosStyle.colorAccent
                leftPadding: 5
            }
            Row {
                id: rw
                anchors.right: parent.right
                spacing: 3

                readonly property real rowHeight: userList.delegateHeight*0.3
                readonly property real fontHeight: rowHeight*0.9
                readonly property int fontWeight: Font.DemiBold


                Row {
                    visible: model && model.d3
                    QTrophyImage {
                        level: 3
                        isDeathmatch: true
                        anchors.verticalCenter: parent.verticalCenter
                        height: rw.rowHeight
                        width: rw.rowHeight
                    }

                    QLabel {
                        anchors.verticalCenter: parent.verticalCenter
                        text: model ? model.d3 : ""
                        font.weight: rw.fontWeight
                        font.pixelSize: rw.fontHeight
                        visible: _showNumbers
                    }
                }

                Row {
                    visible: model && model.t3
                    QTrophyImage {
                        level: 3
                        isDeathmatch: false
                        anchors.verticalCenter: parent.verticalCenter
                        height: rw.rowHeight
                        width: rw.rowHeight
                    }

                    QLabel {
                        anchors.verticalCenter: parent.verticalCenter
                        text: model ? model.t3 : ""
                        font.weight: rw.fontWeight
                        font.pixelSize: rw.fontHeight
                        visible: _showNumbers
                    }
                }

                Row {
                    visible: model && model.d2
                    QTrophyImage {
                        level: 2
                        isDeathmatch: true
                        anchors.verticalCenter: parent.verticalCenter
                        height: rw.rowHeight
                        width: rw.rowHeight
                    }

                    QLabel {
                        anchors.verticalCenter: parent.verticalCenter
                        text: model ? model.d2 : ""
                        font.weight: rw.fontWeight
                        font.pixelSize: rw.fontHeight
                        visible: _showNumbers
                    }
                }

                Row {
                    visible: model && model.t2
                    QTrophyImage {
                        level: 2
                        isDeathmatch: false
                        anchors.verticalCenter: parent.verticalCenter
                        height: rw.rowHeight
                        width: rw.rowHeight
                    }

                    QLabel {
                        anchors.verticalCenter: parent.verticalCenter
                        text: model ? model.t2 : ""
                        font.weight: rw.fontWeight
                        font.pixelSize: rw.fontHeight
                        visible: _showNumbers
                    }
                }

                Row {
                    visible: model && model.d1
                    QTrophyImage {
                        level: 1
                        isDeathmatch: true
                        anchors.verticalCenter: parent.verticalCenter
                        height: rw.rowHeight
                        width: rw.rowHeight
                    }

                    QLabel {
                        anchors.verticalCenter: parent.verticalCenter
                        text: model ? model.d1 : ""
                        font.weight: rw.fontWeight
                        font.pixelSize: rw.fontHeight
                        visible: _showNumbers
                    }
                }

                Row {
                    visible: model && model.t1
                    QTrophyImage {
                        level: 1
                        isDeathmatch: false
                        anchors.verticalCenter: parent.verticalCenter
                        height: rw.rowHeight
                        width: rw.rowHeight
                    }

                    QLabel {
                        anchors.verticalCenter: parent.verticalCenter
                        text: model ? model.t1 : ""
                        font.weight: rw.fontWeight
                        font.pixelSize: rw.fontHeight
                        visible: _showNumbers
                    }
                }


            }
        }


        footer: QToolButtonFooter {
            width: userList.width
            //visible: iconEmpty._visible && modelUserList.count == 0
            action: actionUserEdit
            color: CosStyle.colorAccent
        }


        model: SortFilterProxyModel {
            id: userProxyModel
            sourceModel: modelUserList

            sorters: [
                StringSorter { roleName: "classname"; priority: 2 },
                StringSorter { roleName: "name"; priority: 1 }
            ]

            proxyRoles: [
                ExpressionRole {
                    name: "name"
                    expression: model.firstname+" "+model.lastname
                },
                SwitchRole {
                    name: "titlecolor"
                    filters: ValueFilter {
                        roleName: "activeClient"
                        value: true
                        SwitchRole.value: CosStyle.colorOK
                    }
                    defaultValue: CosStyle.colorPrimaryLighter
                }
            ]
        }

        modelTitleRole: "name"
        modelSubtitleRole: "nickname"
        modelTitleColorRole: "titlecolor"
        modelSubtitleColorRole: "titlecolor"

        highlightCurrentItem: false

        onRefreshRequest: teacherGroups.send("groupGet", { id: teacherGroups.selectedGroupId })

        onClicked: {
            var o = userList.model.get(index)
            control.tabPage.pushContent(componentUserScore, {
                                            username: o.username,
                                            contentTitle: o.name+" | "+teacherGroups.selectedGroupName
                                        })
        }
    }

    QIconEmpty {
        id: iconEmpty
        visible: modelUserList.count == 0
        anchors.centerIn: parent
        textWidth: parent.width*0.75
        text: qsTr("Egyetlen résztvevő sincs még ebben a csoportban")
        tabContainer: control
    }

    Connections {
        target: teacherGroups

        function onGroupGet(jsonData, binaryData) {
            if (!jsonData || jsonData.id !== teacherGroups.selectedGroupId)
                return

            JS.listModelReplace(modelUserList, jsonData.userList)
        }
    }

    onPopulated: teacherGroups.send("groupGet", { id: teacherGroups.selectedGroupId })


    Component {
        id: componentUserEdit
        TeacherGroupUserEdit {  }
    }

    Component {
        id: componentUserScore
        TeacherGroupScore { }
    }


    Action {
        id: actionUserEdit
        text: qsTr("Résztvevők szerkesztése")
        icon.source: "qrc:/internal/icon/pencil.svg"
        enabled: teacherGroups.selectedGroupId > -1
        onTriggered: {
            control.tabPage.pushContent(componentUserEdit)
        }
    }
}



