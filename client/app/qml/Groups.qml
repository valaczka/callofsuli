import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
    id: control

    title: qsTr("Áttekintés")
    icon: "qrc:/internal/icon/speedometer.svg"

    property Profile profile: null

    property real groupsButtonSize: CosStyle.twoLineHeight*2.75

    menu: QMenu {
        MenuItem { action: actionQRinfo }
        QMenu {
            title: qsTr("Betűméret")
            MenuItem { action: mainWindow.actionFontPlus }
            MenuItem { action: mainWindow.actionFontMinus }
            MenuItem { action: mainWindow.actionFontReset }
        }
    }


    Flickable {
        id: flickable
        width: parent.width
        height: Math.min(parent.height, contentHeight)
        anchors.centerIn: parent

        contentWidth: col.width
        contentHeight: col.height

        clip: true

        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        ScrollIndicator.vertical: ScrollIndicator { }

        Column {
            id: col
            width: flickable.width

            spacing: 10

            ProfileDetailsUser {
                id: user

                topPadding: control.tabPage.headerPadding+10
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width

                bottomPadding: (cosClient.userRoles & Client.RoleTeacher) ? 40 : 20
            }

            ProfileDetailsProgress {
                id: barXP
                width: parent.width*0.75
                anchors.horizontalCenter: parent.horizontalCenter
                textFormat: "%1 XP"
                color: CosStyle.colorOKLighter

                visible: (cosClient.userRoles & Client.RoleStudent)

                property int rank: -1

                onRankChanged: {
                    var n = cosClient.nextRank(rank)
                    if (!Object.keys(n).length) {
                        textTo = ""
                        to = 0
                        from = 0
                    } else {
                        to = n.next.xp
                        from = n.current.xp
                        textTo = n.next.rankname+(n.next.ranklevel > 0 ? " (lvl "+n.next.ranklevel+")" : "")
                    }
                }
            }

            ProfileDetailsProgress {
                id: barStreak
                width: parent.width*0.75
                anchors.horizontalCenter: parent.horizontalCenter
                textFormat: qsTr("Széria: %1")
                valueToVisible: false
                color: CosStyle.colorAccent

                visible: (cosClient.userRoles & Client.RoleStudent)

                bottomPadding: 50
            }


            Grid {
                id: groupsGrid
                anchors.horizontalCenter: parent.horizontalCenter
                columns: Math.floor(parent.width/(groupsButtonSize+spacing))
                spacing: groupsButtonSize*0.2

                bottomPadding: 20

                Repeater {
                    id: groupRepeater

                    QCard {
                        id: groupItem
                        height: groupsButtonSize
                        width: groupsButtonSize

                        backgroundColor: (cosClient.userRoles & Client.RoleTeacher) ? "#996202" : "#4d0299"

                        onClicked: {
                            if (cosClient.userRoles & Client.RoleTeacher)
                                JS.createPage("TeacherGroup", {
                                                  groupId: modelData.id,
                                                  allGroupList: groupRepeater.model
                                              })
                            else
                                JS.createPage("StudentGroup", {
                                                  title: modelData.name+(modelData.readableClassList !== "" ? " | "+modelData.readableClassList : ""),
                                                  groupId: modelData.id,
                                                  profile: profile
                                              })
                        }

                        Column {
                            anchors.centerIn: parent

                            QFontImage {
                                id: image
                                anchors.horizontalCenter: parent.horizontalCenter
                                size: groupItem.width*0.5
                                color: "white"
                                visible: icon
                                icon: "qrc:/internal/icon/account-group.svg"
                            }

                            QLabel {
                                id: title
                                color: "white"
                                font.weight: Font.DemiBold
                                width: groupItem.width*0.8
                                horizontalAlignment: Text.AlignHCenter
                                wrapMode: Text.Wrap
                                maximumLineCount: 2
                                elide: Text.ElideRight
                                font.capitalization: Font.AllUppercase
                                text: modelData.name
                            }

                            QLabel {
                                id: subtitle
                                color: "white"
                                font.weight: Font.Light
                                font.pixelSize: CosStyle.pixelSize*0.9
                                width: groupItem.width*0.8
                                horizontalAlignment: Text.AlignHCenter
                                maximumLineCount: 2
                                elide: Text.ElideRight
                                text: modelData.readableClassList
                            }
                        }
                    }
                }

                QCard {
                    id: cardAdd
                    height: groupsButtonSize
                    width: groupsButtonSize

                    visible: (cosClient.userRoles & Client.RoleTeacher)

                    backgroundColor: CosStyle.colorOKDarker

                    onClicked: {
                        var d = JS.dialogCreateQml("TextField", { title: qsTr("Új csoport létrehozása"), text: qsTr("Új csoport neve:") })

                        d.accepted.connect(function(data) {
                            if (data.length && profile)
                                profile.send(CosMessage.ClassTeacher, "groupCreate", {name: data})
                        })
                        d.open()
                    }

                    Column {
                        anchors.centerIn: parent

                        QFontImage {
                            anchors.horizontalCenter: parent.horizontalCenter
                            size: cardAdd.width*0.5
                            color: "white"
                            visible: icon
                            icon: "qrc:/internal/icon/account-multiple-plus-outline.svg"
                        }

                        QLabel {
                            anchors.horizontalCenter: parent.horizontalCenter
                            color: "white"
                            font.weight: Font.DemiBold
                            width: cardAdd.width*0.8
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.Wrap
                            maximumLineCount: 3
                            elide: Text.ElideRight
                            font.capitalization: Font.AllUppercase
                            text: qsTr("Új csoport létrehozása")
                        }
                    }
                }
            }

            QButton {
                anchors.horizontalCenter: parent.horizontalCenter
                visible: (cosClient.userRoles & Client.RoleTeacher)
                text: qsTr("Pályák kezelése")
                icon.source: "qrc:/internal/icon/briefcase-variant.svg"
                display: AbstractButton.TextBesideIcon
                onClicked: JS.createPage("TeacherMap", {})
                themeColors: CosStyle.buttonThemeOrange
                padding: 20
            }

            QIconEmpty {
                visible: !groupRepeater.model || groupRepeater.model.length === 0
                anchors.horizontalCenter: parent.horizontalCenter
                textWidth: col.width*0.75
                topPadding: 50
                bottomPadding: 50
                text: qsTr("Sajnos egyetlen csoportnak sem vagy még tagja")
                tabContainer: control
            }

        }

    }

    onPopulated: {
        profile.send("getUser", {username: cosClient.userName})
        profile.send("getMyGroups")
    }

    Connections {
        target: control.tabPage

        function onPageActivated() {
            profile.send("getMyGroups")
        }

    }


    Connections {
        target: profile

        function onGetUser(jsonData, binaryData) {
            if (jsonData.username !== cosClient.userName || control.StackView.status !== StackView.Active)
                return

            user.userName = (jsonData.nickname && jsonData.nickname !== "" ?
                                 jsonData.nickname :
                                 jsonData.firstname+" "+jsonData.lastname)

            user.picture = jsonData.picture
            user.rankId = Number(jsonData.rankid)
            user.rankLevel = Number(jsonData.ranklevel)
            user.rankImage = jsonData.rankimage
            user.rankName = jsonData.rankname

            if (cosClient.userRoles & Client.RoleStudent) {
                barXP.rank  = jsonData.rankid
                barXP.value = jsonData.xp
                barStreak.value = Number(jsonData.currentStreak)

                if (jsonData.maxStreak) {
                    barStreak.textTo = jsonData.maxStreak>jsonData.currentStreak ?
                                qsTr("leghosszabb széria: %1").arg(jsonData.maxStreak) :
                                Number(jsonData.currentStreak)+1
                    barStreak.to = Math.max(jsonData.maxStreak, Number(jsonData.currentStreak)+1)
                }
            }
        }

        function onGetMyGroups(jsonData, binaryData) {
            groupRepeater.model = jsonData.list
        }

        function onGroupCreate(jsonData, binaryData) {
            if ((cosClient.userRoles & Client.RoleTeacher) && jsonData.created) {
                JS.createPage("TeacherGroup", {
                                  groupId: Number(jsonData.created),
                                  allGroupList: groupRepeater.model
                              })
            }
        }
    }


    Component {
        id: cmpServerInfo
        ServerInfo {}
    }

    Action {
        id: actionQRinfo
        icon.source: "qrc:/internal/icon/qrcode.svg"
        text: qsTr("Csatlakozási info")
        onTriggered: tabPage.pushContent(cmpServerInfo, {})
    }

}
