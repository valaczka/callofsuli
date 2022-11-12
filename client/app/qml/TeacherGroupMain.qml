import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
    id: control

    title: teacherGroups.selectedGroupFullName
    icon: "qrc:/internal/icon/account-group.svg"

    property real buttonSize: CosStyle.twoLineHeight*2.75
    readonly property real _contentWidth: Math.min(width-10, 700)

    menu: QMenu {
        MenuItem { action: actionGroupRename }
        MenuItem { action: actionGroupDelete}
    }

    Flickable {
        id: flickable
        width: parent.width
        height: Math.min(parent.height, contentHeight)
        anchors.centerIn: parent

        contentWidth: gridColumn.width
        contentHeight: gridColumn.height

        clip: true

        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        ScrollIndicator.vertical: ScrollIndicator { }

        Column {
            id: gridColumn
            width: flickable.width

            QLabel {
                text: teacherGroups.selectedGroupFullName

                font.pixelSize: CosStyle.pixelSize*1.7
                font.weight: Font.Normal
                color: CosStyle.colorAccentLight
                width: parent.width*0.8
                wrapMode: Text.Wrap
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter

                topPadding: grid.height >= control.height-200 ? control.tabPage.headerPadding : 0
                bottomPadding: 50
            }


            Grid {
                id: grid
                columns: Math.floor(_contentWidth/(buttonSize+spacing))
                spacing: buttonSize*0.2

                anchors.horizontalCenter: parent.horizontalCenter

                bottomPadding: 20

                Repeater {
                    id: groupRepeater

                    model: ListModel {
                        ListElement {
                            title: qsTr("Résztvevők")
                            icon: "image://font/School/\uf154"
                            func: function() { control.tabPage.pushContent(cmpTeacherGroupUserList) }
                        }
                        ListElement {
                            title: qsTr("Pályák")
                            icon: "image://font/Academic/\uf122"
                            textcolor: "white"
                            background: "lime"
                            func: function() { control.tabPage.pushContent(cmpTeacherGroupMapList) }
                        }
                        ListElement {
                            title: qsTr("Hadjáratok")
                            icon: "qrc:/internal/icon/calendar-multiple.svg"
                            func: function() { control.tabPage.pushContent(cmpTeacherCampaign) }
                        }
                        ListElement {
                            title: qsTr("Dolgozatok")
                            icon: "image://font/AcademicI/\uf15d"
                            func: function() { control.tabPage.pushContent(cmpTeacherGroupExamList) }
                        }
                    }

                    QCard {
                        id: groupItem
                        height: buttonSize
                        width: buttonSize

                        required property string icon
                        required property string title
                        required property var func

                        backgroundColor: "lime"
                        property color textcolor: "white"

                        onClicked: {
                            func()
                            //JS.createPage("Admin", { page: page })
                        }

                        Column {
                            anchors.centerIn: parent

                            QFontImage {
                                id: image
                                anchors.horizontalCenter: parent.horizontalCenter
                                size: groupItem.width*0.5
                                color: groupItem.textcolor
                                visible: icon
                                icon: groupItem.icon
                            }

                            QLabel {
                                id: title
                                color: groupItem.textcolor
                                font.weight: Font.DemiBold
                                width: groupItem.width*0.8
                                horizontalAlignment: Text.AlignHCenter
                                wrapMode: Text.Wrap
                                maximumLineCount: 2
                                elide: Text.ElideRight
                                font.capitalization: Font.AllUppercase
                                text: groupItem.title
                            }

                        }
                    }
                }

            }


        }
    }



    Component {
        id: cmpTeacherGroupMapList
        TeacherGroupMapList { }
    }

    Component {
        id: cmpTeacherGroupUserList
        TeacherGroupUserList { }
    }

    Component {
        id: cmpTeacherGroupExamList
        TeacherGroupExamList { }
    }

    Component {
        id: cmpTeacherGroupExamEngine
        TeacherGroupExamEngine { }
    }

    Component {
        id: cmpTeacherCampaign
        TeacherGroupCampaign { }
    }



    Action {
        id: actionGroupDelete
        text: qsTr("Csoport törlése")
        icon.source: "qrc:/internal/icon/delete.svg"
        enabled: teacherGroups.selectedGroupId > -1
        onTriggered: {
            var d = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan törlöd a csoportot?\n%1").arg(teacherGroups.selectedGroupFullName)})
            d.accepted.connect(function() {
                teacherGroups.send("groupRemove", {id: teacherGroups.selectedGroupId})
            })
            d.open()
        }
    }

    Action {
        id: actionGroupRename
        text: qsTr("Átnevezés")
        icon.source: CosStyle.iconRename
        enabled: teacherGroups.selectedGroupId > -1
        onTriggered: {
            var d = JS.dialogCreateQml("TextField", {
                                           title: qsTr("Csoport átnevezése"),
                                           text: qsTr("Csoport neve:"),
                                           value: teacherGroups.selectedGroupName
                                       })

            d.accepted.connect(function(data) {
                if (data.length)
                    teacherGroups.send("groupModify", {id: teacherGroups.selectedGroupId, name: data})
            })
            d.open()
        }
    }

    onPopulated: {
    }

}
