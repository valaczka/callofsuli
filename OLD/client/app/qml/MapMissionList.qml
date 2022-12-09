import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
    id: control

    title: qsTr("Küldetések")
    icon: CosStyle.iconMedal

    maximumWidth: 800

    property StudentMaps studentMaps: null
    property Action actionLite: null

    property ListModel missionListModel: ListModel {}

    property real medalButtonSize: CosStyle.twoLineHeight*2.8

    signal missionLevelLoad(string uuid, int level, bool deathmatch)

    menu: actionLite ? menu1 : null

    QMenu {
        id: menu1
        MenuItem { action: actionLite }
    }

    SortFilterProxyModel {
        id: missionProxyModel
        sourceModel: missionListModel
        sorters: [
            FilterSorter {
                ValueFilter { roleName: "lockDepth"; value: 0 }
                priority: 2
            },
            RoleSorter { roleName: "num"; priority: 1 },
            StringSorter { roleName: "name"; priority: 0 }
        ]
        /*proxyRoles: [
            SwitchRole {
                name: "textColor"
                filters: [
                    ExpressionFilter {
                        expression: model.lockDepth === 0 && model.fullSolved
                        SwitchRole.value: CosStyle.colorOKLighter
                    },
                    ExpressionFilter {
                        expression: model.lockDepth > 0
                        SwitchRole.value: JS.setColorAlpha(CosStyle.colorPrimary, 0.5)
                    }
                ]
                defaultValue: CosStyle.colorPrimary
            }
        ]*/
    }

    Flickable {
        id: flick

        anchors.fill: parent
        flickableDirection: Flickable.VerticalFlick

        contentWidth: col.width
        contentHeight: col.height

        boundsBehavior: Flickable.StopAtBounds


        ScrollIndicator.vertical: ScrollIndicator { }

        Column {
            id: col
            width: flick.width

            //spacing: CosStyle.pixelSize*4

            QTabHeader {
                tabContainer: control
                flickable: flick
            }

            Repeater {
                id: rptr

                model: missionProxyModel

                Column {
                    id: col2
                    required property var model
                    required property int index

                    QLabel {
                        text: col2.model.name
                        width: col.width
                        wrapMode: Text.Wrap
                        font.family: "HVD Peace"
                        font.pixelSize: CosStyle.pixelSize*1.4
                        color: col2.model.fullSolved ? CosStyle.colorOKLighter : CosStyle.colorPrimary
                        topPadding: col2.index == 0 ? 0 : CosStyle.pixelSize*2
                        leftPadding: Math.max(10, mainWindow.safeMarginLeft)
                        rightPadding: Math.max(10, mainWindow.safeMarginRight)
                        bottomPadding: 5
                    }

                    ListView {
                        id: view
                        model: col2.model.levels
                        orientation: ListView.Horizontal

                        width: col.width
                        height: medalButtonSize
                        spacing: 0

                        clip: true

                        header: Item {
                            width: mainWindow.safeMarginLeft
                            height: view.height
                        }

                        footer: Item {
                            width: mainWindow.safeMarginRight
                            height: view.height
                        }

                        delegate: QCard {
                            id: cardDelegate
                            height: medalButtonSize
                            width: medalButtonSize*0.95

                            required property var model

                            color: "transparent"
                            border.color: "transparent"

                            onClicked: missionLevelLoad(col2.model.uuid, model.level, model.deathmatch)

                            readonly property color contentColor: model.solved ? (model.level === 3 ? "#d3ec66" :
                                                                                                      (model.level === 2 ? "#ee9100" : "#02e452")) :
                                                                                 model.available ? CosStyle.colorAccentLighter :
                                                                                                   CosStyle.colorPrimaryDark

                            Column {
                                anchors.centerIn: parent
                                spacing: 7

                                QLabel {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    color: cardDelegate.contentColor
                                    font.weight: Font.Medium
                                    font.pixelSize: CosStyle.pixelSize*0.7
                                    horizontalAlignment: Text.AlignHCenter
                                    font.capitalization: Font.AllUppercase
                                    text: cardDelegate.model.deathmatch ?
                                              "Level %1 Sudden death".arg(cardDelegate.model.level) :
                                              "Level %1".arg(cardDelegate.model.level)
                                    visible: cardDelegate.model.available
                                }

                                QFontImage {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    size: cardDelegate.height*0.3
                                    color: CosStyle.colorPrimaryDarker
                                    visible: !cardDelegate.model.available
                                    icon: CosStyle.iconLock
                                    opacity: 0.7
                                }

                                QMedalImage {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    visible: cardDelegate.model.available
                                    level: cardDelegate.model.solved ? cardDelegate.model.level : -1
                                    image: cardDelegate.model.solved ? col2.model.medalImage : ""
                                    isDeathmatch: cardDelegate.model.deathmatch
                                    height: cardDelegate.height*0.6
                                    width: height
                                }


                                QLabel {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    color: cardDelegate.contentColor
                                    font.weight: Font.DemiBold
                                    font.pixelSize: CosStyle.pixelSize
                                    text: "%1 XP".arg(cardDelegate.model.xp)
                                }
                            }
                        }
                    }

                    Connections {
                        target: mainWindow
                        function onSafeMarginsChanged() {
                            view.positionViewAtBeginning()
                        }
                    }

                }


            }
        }

    }


    Connections {
        target: studentMaps

        function onSolvedMissionListReady(list) {
            JS.listModelReplace(missionListModel, list)
        }
    }

    onPopulated: {
        if (studentMaps)
            studentMaps.getMissionList()
    }

}



