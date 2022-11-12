import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
    id: control

    title: qsTr("Hadjárat eredményei")
    icon: "qrc:/internal/icon/calendar-edit.svg"

    property int campaignId: -1
    property bool finished: true
    property bool componentDetailsEnabled: true

    menu: QMenu {
        MenuItem {
            text: qsTr("Mindet kinyit")
            icon.source: CosStyle.iconAdd
            onClicked: {
                for (var i=0; i<rptrResult.count; ++i)
                    rptrResult.itemAt(i).collapsed = false
            }
        }

        MenuItem {
            text: qsTr("Mindet becsuk")
            icon.source: CosStyle.iconAdd
            onClicked: {
                for (var i=0; i<rptrResult.count; ++i)
                    rptrResult.itemAt(i).collapsed = true
            }
        }

        MenuSeparator {}

        MenuItem {
            text: qsTr("Szerkesztés")

            enabled: componentDetailsEnabled
            icon.source: CosStyle.iconAdd
            onClicked: {
                control.tabPage.pushContent(control.tabPage.cmpTeacherCampaignDetails, {
                                                campaignId: control.campaignId,
                                                contentTitle: control.contentTitle,
                                                componentResultEnabled: false
                                            })
            }
        }
    }


    ListModel {
        id: modelResult
    }

    QAccordion {
        id: acc

        QTabHeader {
            tabContainer: control
            flickable: acc.flickable
        }

        Repeater {
            id: rptrResult

            model: SortFilterProxyModel {
                sourceModel: modelResult

                sorters: [
                    RoleSorter { roleName: "active"; priority: 3; sortOrder: Qt.DescendingOrder },
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
                        filters: [
                            ValueFilter {
                                roleName: "active"
                                value: false
                                SwitchRole.value: CosStyle.colorPrimaryDark
                            },
                            ValueFilter {
                                roleName: "active"
                                value: 0
                                SwitchRole.value: CosStyle.colorPrimaryDark
                            }
                        ]
                        defaultValue: CosStyle.colorAccentLighter
                    }
                ]
            }

            QCollapsible {
                id: cl

                required property var assignment
                required property string name
                required property color titlecolor
                required property string classname

                property string assignmentString: ""
                property string forecastString: ""

                title: name
                collapsed: true
                titleColor: titlecolor

                rightComponent: Row {
                    rightPadding: Math.max(mainWindow.safeMarginRight, 5)

                    QLabel {
                        anchors.verticalCenter: parent.verticalCenter
                        text: cl.forecastString
                        font.weight: Font.DemiBold
                        color: CosStyle.colorOKLighter
                        font.pixelSize: CosStyle.pixelSize
                        leftPadding: 5
                        rightPadding: 5
                    }

                    QLabel {
                        anchors.verticalCenter: parent.verticalCenter
                        text: cl.assignmentString
                        font.weight: Font.DemiBold
                        color: CosStyle.colorWarningLight
                        font.pixelSize: CosStyle.pixelSize
                        leftPadding: 5
                        rightPadding: 5
                    }
                }

                Campaign {
                    title: cl.name
                    subtitle: cl.classname
                    assignment: cl.assignment
                    finished: control.finished
                    //campaingDetails: false

                    activity: teacherGroups

                    width: parent.width
                }

                Component.onCompleted: {
                    if (!assignment)
                        return

                    var slist = []
                    var flist = []

                    for (var i=0; i<assignment.count; ++i) {
                        var gl = assignment.get(i).grades

                        if (!gl)
                            continue

                        for (var j=0; j<gl.count; ++j) {
                            var g = gl.get(j)

                            if (g.forecast) {
                                if (g.gradeid !== -1) {
                                    flist.push(teacherGroups.grade(g.gradeid).shortname)
                                } else if (g.xp > 0) {
                                    flist.push("%1 XP".arg(g.xp))
                                }
                            } else {
                                if (g.gradeid !== -1) {
                                    slist.push(teacherGroups.grade(g.gradeid).shortname)
                                } else if (g.xp > 0) {
                                    slist.push("%1 XP".arg(g.xp))
                                }
                            }
                        }
                    }

                    assignmentString = slist.join(" | ")
                    forecastString = flist.join(" | ")
                }
            }
        }

    }



    Connections {
        target: teacherGroups


        function onCampaignResultGetReady(id, list) {
            if (id !== campaignId)
                return

            JS.listModelReplace(modelResult, list)
        }
    }

    onPopulated: {
        teacherGroups.send("campaignResultGet", {id: campaignId})
    }



}



