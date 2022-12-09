import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QCollapsible {
    id: control

    required property bool selected

    property GameMapEditorMission self: null

    property bool editable: true

    backgroundColor: "transparent"
    titleColor: CosStyle.colorWarningLighter
    lineColor: "transparent"
    itemSelected: selected

    signal missionRemove()

    title: self && (selected || collapsed)? self.name : ""

    rightComponent: Row {
        spacing: 0
        /*QToolButton {
            anchors.verticalCenter: parent.verticalCenter
            action: actionMissionEdit
            color: CosStyle.colorAccent
            display: AbstractButton.IconOnly
            visible: !control.collapsed && !control.editable
        }*/
        QToolButton {
            anchors.verticalCenter: parent.verticalCenter
            action: actionMissionRemove
            color: CosStyle.colorErrorLighter
            display: AbstractButton.IconOnly
        }
    }

    QMenu {
        id: contextMenu

        MenuItem { action: actionMissionRemove }
    }

    onRightClicked: contextMenu.open()

    Column {
        id: controlContent
        width: parent.width

        QGridLayout {
            id: contentLayout
            watchModification: false

            isFullscreen: control.compact

            QGridImageTextField {
                id: imageTextName
                fieldName: qsTr("Küldetés neve")

                textfield.font.family: "Rajdhani"
                textfield.font.pixelSize: CosStyle.pixelSize*1.5
                textfield.textColor: CosStyle.colorWarningLighter
                textfield.readOnly: !control.editable
                textfield.onTextModified: mapEditor.missionModify(self, {name: textfield.text})

                text: self ? self.name : ""

                image: self && self.medalImage != "" ? cosClient.medalIconPath(self.medalImage) : ""

                mousearea.enabled: control.editable
                mousearea.onClicked:  {
                    var d = JS.dialogCreateQml("ImageGrid", {
                                                   model: cosClient.medalIcons(),
                                                   icon: CosStyle.iconMedal,
                                                   title: qsTr("Küldetés medálképe"),
                                                   modelImagePattern: "qrc:/internal/medals/%1",
                                                   clearEnabled: false,
                                                   currentValue: self.medalImage
                                               })

                    d.accepted.connect(function(data) {
                        if (data)
                            mapEditor.missionModify(self, {medalImage: data})
                    })

                    d.open()
                }
            }

            QGridLabel {
                field: areaDetails
            }

            QGridTextArea {
                id: areaDetails
                fieldName: qsTr("Leírás")
                placeholderText: qsTr("Rövid ismertető leírás a küldetésről")
                minimumHeight: CosStyle.baseHeight*2
                readOnly: !control.editable
                color: CosStyle.colorWarning

                text: self ? self.description : ""

                onTextModified: mapEditor.missionModify(self, {description: text})
            }
        }


        Row {
            id: levelRow
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 5

            topPadding: 10
            bottomPadding: 20

            Repeater {
                id: groupRepeater

                model: self ? self.levels : null

                QCard {
                    id: levelItem
                    height: CosStyle.pixelSize*4.5
                    width: height

                    backgroundColor: CosStyle.colorAccent

                    required property int index

                    property GameMapEditorMissionLevel levelSelf: self.levels.object(index)

                    onLevelSelfChanged: if (!levelSelf) {
                                            delete levelItem
                                        }

                    onClicked: {
                        mapEditor.openMissionLevel({
                                                       missionLevel: levelSelf
                                                   })
                    }

                    Column {
                        anchors.centerIn: parent
                        spacing: 0

                        QLabel {
                            anchors.horizontalCenter: parent.horizontalCenter
                            color: "white"
                            font.weight: Font.DemiBold
                            font.pixelSize: CosStyle.pixelSize*2
                            text: levelItem.levelSelf ? levelItem.levelSelf.level : ""
                        }

                        QLabel {
                            anchors.horizontalCenter: parent.horizontalCenter
                            color: "white"
                            font.weight: Font.Medium
                            font.pixelSize: CosStyle.pixelSize*0.9
                            font.capitalization: Font.AllUppercase
                            text: "Level"
                        }
                    }
                }
            }

            QCard {
                id: levelAddItem
                height: CosStyle.pixelSize*4.5
                width: height

                visible: self && self.levels.count < 3

                backgroundColor: CosStyle.colorOKDark

                QLabel {
                    anchors.centerIn: parent
                    color: CosStyle.colorOKLighter
                    font.weight: Font.Bold
                    font.pixelSize: CosStyle.pixelSize*3
                    text: "+"
                }

                onClicked: mapEditor.missionLevelAdd(self, {})

            }
        }



        Rectangle {
            id: lockBgRect

            width: parent.width
            height: lockRow.height

            color: JS.setColorAlpha(CosStyle.colorErrorDark, 0.3)

            Row {
                id: lockRow

                width: parent.width

                QFontImage {
                    id: lockImage
                    size: CosStyle.pixelSize*2
                    icon: CosStyle.iconLock
                    color: CosStyle.colorErrorLighter
                    width: size*2
                    anchors.verticalCenter: parent.verticalCenter
                }


                QObjectListView {
                    id: list

                    anchors.verticalCenter: parent.verticalCenter

                    width: parent.width-lockImage.width

                    model: SortFilterProxyModel {
                        sourceModel: self ? self.locks : null

                        proxyRoles: ExpressionRole {
                            name: "name"
                            expression: model.mission.name
                        }

                        sorters: StringSorter {
                            roleName: "name"
                        }
                    }

                    modelTitleRole: "name"
                    colorTitle: CosStyle.colorAccentLighter

                    autoSelectorChange: false
                    refreshEnabled: false

                    delegateHeight: CosStyle.baseHeight


                    leftComponent: Item {
                        width: list.delegateHeight
                        height: width

                        QBadge {
                            anchors.centerIn: parent
                            text: model && model.level ? model.level : ""
                            color: CosStyle.colorWarningDarker
                        }
                    }

                    rightComponent: QToolButton {
                        anchors.verticalCenter: parent.verticalCenter
                        icon.source: "qrc:/internal/icon/delete.svg"
                        color: CosStyle.colorErrorLighter
                        onClicked: {
                            mapEditor.missionLockRemove(self, list.modelObject(modelIndex))
                        }
                    }


                    footer: QToolButtonFooter {
                        width: list.width
                        color: CosStyle.colorAccentLight
                        text: qsTr("Zárolás hozzáadása")
                        icon.source: CosStyle.iconAdd
                        onClicked: {
                            mapEditor.updateMissionLevelModelMission(self)

                            if (mapEditor.missionLevelModel.count < 1) {
                                cosClient.sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", qsTr("Zárolások"), qsTr("Nincs hozzáadható küldetés!"))
                                return
                            }


                            var d = JS.dialogCreateQml("MissionList", {
                                                           icon: CosStyle.iconLockAdd,
                                                           title: qsTr("%1 - Zárolás").arg(self.name),
                                                           selectorSet: false,
                                                           sourceModel: mapEditor.missionLevelModel
                                                       })

                            d.accepted.connect(function(dlgdata) {
                                if (dlgdata < 0)
                                    return

                                mapEditor.missionLockAdd(self, d.item.list.modelObject(dlgdata).missionLevel)
                            })
                            d.open()
                        }
                    }


                    onClicked: {
                        var o = modelObject(index)

                        mapEditor.updateMissionLevelModelLock(o)

                        if (mapEditor.missionLevelModel.count < 1) {
                            cosClient.sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", qsTr("Zárolás"), qsTr("A kiválasztott szintet nincs mire módosítani!"))
                            return
                        }


                        var d = JS.dialogCreateQml("MissionList", {
                                                       icon: CosStyle.iconLockAdd,
                                                       title: qsTr("%1 - Zárolás").arg(self.name),
                                                       selectorSet: false,
                                                       sourceModel: mapEditor.missionLevelModel
                                                   })

                        d.accepted.connect(function(dlgdata) {
                            if (dlgdata < 0)
                                return

                            mapEditor.missionLockReplace(self, o, d.item.list.modelObject(dlgdata).missionLevel)
                        })
                        d.open()
                    }

                }

            }

        }
    }

    Rectangle {
        anchors.bottom: controlContent.bottom
        anchors.left: controlContent.left
        width: controlContent.width
        height: 0.5
        color: Qt.darker(CosStyle.colorAccentDark)
    }



    Action {
        id: actionMissionEdit

        icon.source: "qrc:/internal/icon/pencil.svg"
        text: qsTr("Szerkesztés")

        onTriggered: control.editable = true
    }

    Action {
        id: actionMissionRemove

        icon.source: "qrc:/internal/icon/delete.svg"
        text: qsTr("Törlés")

        onTriggered: control.missionRemove()
    }
}
