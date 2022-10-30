import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
    id: control

    title: qsTr("Felhasználók")
    icon: CosStyle.iconUsers

    property var queryParameters: ({})

    menu: QMenu {
        MenuItem { action: actionRename }
        MenuItem { action: actionDelete }
        MenuSeparator {}
        MenuItem { action: actionQR }
        MenuItem { action: actionQRgoogle }
    }

    ListModel {
        id: _filteredClassModel
    }

    SortFilterProxyModel {
        id: userProxyModel
        sourceModel: serverSettings.modelUserList

        sorters: [
            RoleSorter { roleName: "active"; sortOrder:Qt.DescendingOrder; priority: 3 },
            StringSorter { roleName: "firstname"; sortOrder: Qt.AscendingOrder; priority: 2 },
            StringSorter { roleName: "lastname"; sortOrder: Qt.AscendingOrder; priority: 1 }
        ]

        proxyRoles: [
            ExpressionRole {
                name: "name"
                expression: model.firstname+" "+model.lastname
            },
            ExpressionRole {
                name: "details"
                expression: model.nickname === "" ? model.username : model.username+" - "+model.nickname
            },
            SwitchRole {
                name: "background"
                filters: [
                    ValueFilter {
                        roleName: "active"
                        value: false
                        SwitchRole.value: "transparent"
                    },
                    ValueFilter {
                        roleName: "isAdmin"
                        value: true
                        SwitchRole.value: JS.setColorAlpha(CosStyle.colorErrorDark, 0.4)
                    },
                    ValueFilter {
                        roleName: "isTeacher"
                        value: true
                        SwitchRole.value: JS.setColorAlpha(CosStyle.colorAccentDark, 0.4)
                    }
                ]
                defaultValue: "transparent"
            },
            SwitchRole {
                name: "titlecolor"
                filters: [
                    ValueFilter {
                        roleName: "active"
                        value: false
                        SwitchRole.value: "#99ffffff"
                    },
                    ValueFilter {
                        roleName: "username"
                        value: cosClient.userName
                        SwitchRole.value: CosStyle.colorAccentLight
                    }
                ]
                defaultValue: CosStyle.colorPrimaryLighter
            },
            SwitchRole {
                name: "subtitlecolor"
                filters: [
                    ValueFilter {
                        roleName: "active"
                        value: false
                        SwitchRole.value: "#55ffffff"
                    },
                    ValueFilter {
                        roleName: "username"
                        value: cosClient.userName
                        SwitchRole.value: CosStyle.colorAccentLighter
                    }
                ]
                defaultValue: CosStyle.colorPrimaryDarker
            }

        ]
    }

    QIconEmpty {
        visible: userProxyModel.count == 0
        anchors.centerIn: parent
        textWidth: parent.width*0.75
        tabContainer: control
        text: qsTr("Egyetlen felhasználó sem tartozik ide")
    }


    QObjectListView {
        id: userList
        anchors.fill: parent

        refreshEnabled: true
        delegateHeight: CosStyle.twoLineHeight
        autoSelectorChange: true

        isFullscreen: control.compact

        header: QTabHeader {
            tabContainer: control
            flickable: userList
        }

        leftComponent: QProfileImage {
            source: model && model.picture ? model.picture : ""
            rankId: model ? model.rankid : -1
            rankImage: model ? model.rankimage : ""
            width: userList.delegateHeight+10
            height: userList.delegateHeight*0.8
            active: model && model.active
        }

        rightComponent: QFontImage {
            icon: "qrc:/internal/img/google.svg"
            visible: model && model.isOauth2
            color: CosStyle.colorPrimaryDarker
            size: userList.delegateHeight*0.4
            width: userList.delegateHeight
        }

        model: userProxyModel
        modelTitleRole: "name"
        modelSubtitleRole: "details"
        modelBackgroundRole: "background"
        modelTitleColorRole: "titlecolor"
        modelSubtitleColorRole: "subtitlecolor"

        highlightCurrentItem: false

        onRefreshRequest: serverSettings.send("userListGet", queryParameters)

        onClicked: {
            //userSelected(userList.modelObject(index).username)
        }

        onRightClicked: contextMenu.popup()
        onLongPressed: contextMenu.popup()

        QMenu {
            id: contextMenu

            MenuItem {
                text: qsTr("Aktívvá tesz")
                icon.source: CosStyle.iconVisible
            }

            MenuItem {
                text: qsTr("Inaktívvá tesz")
                icon.source: CosStyle.iconInvisible
            }

            MenuItem {
                text: qsTr("Töröl")
                icon.source: "qrc:/internal/icon/delete.svg"
            }

            QMenu {
                id: submenu
                title: qsTr("Áthelyez")

                Instantiator {
                    model: _filteredClassModel

                    MenuItem {
                        text: model.name
                        //onClicked: objectiveMoveCopy(model.id, true, item.objectiveSelf)
                    }

                    onObjectAdded: submenu.insertItem(index, object)
                    onObjectRemoved: submenu.removeItem(object)
                }
            }

            MenuSeparator { }
        }

    }



    onPopulated: {
        serverSettings.send("userListGet", queryParameters)
    }


    Connections {
        target: serverSettings

        function onClassUpdate(jsonData, binaryData) {
            if (queryParameters.classid !== undefined && queryParameters.classid !== -1 && jsonData.updated === queryParameters.classid) {
                contentTitle = jsonData.name
                tabPage.contentTitle = contentTitle
            }
        }

        function onUserListGet(jsonData, binaryData) {
            actionQR.classCode = jsonData.code
        }
    }

    Action {
        id: actionRename
        text: qsTr("Átnevezés")
        icon.source: CosStyle.iconRename
        enabled: queryParameters.classid !== undefined && queryParameters.classid > 0
        onTriggered: {
            var d = JS.dialogCreateQml("TextField", { title: qsTr("Osztály neve"), value: control.contentTitle })

            d.accepted.connect(function(data) {
                if (data.length)
                    serverSettings.send("classUpdate", {id: queryParameters.classid, name: data})
            })
            d.open()
        }
    }

    Action {
        id: actionDelete
        text: qsTr("Törlés")
        icon.source: "qrc:/internal/icon/delete.svg"
        enabled: queryParameters.classid !== undefined && queryParameters.classid > 0
        onTriggered: {
            var d = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan törlöd az osztályt?\n%1").arg(control.contentTitle)})
            d.accepted.connect(function() {
                serverSettings.send("classRemove", {id: queryParameters.classid})
                mainStack.back()
            })
            d.open()
        }
    }


    Action {
        id: actionToTeacher
        text: qsTr("Tanárrá tesz")
        icon.source: "image://font/Academic/\uf213"
    }

    Action {
        id: actionToStudent
        text: qsTr("Diákká tesz")
        icon.source: CosStyle.iconUser
    }

    Action {
        id: actionToAdmin
        text: qsTr("Adminná tesz")
        icon.source: "image://font/AcademicI/\uf1ec"
    }

    Action {
        id: actionRevokeAdmin
        text: qsTr("Admint visszavon")
        icon.source: "image://font/Academic/\uf213"
    }


    Action {
        id: actionQR
        icon.source: CosStyle.iconComputerData
        text: qsTr("Regisztrációs info")
        enabled: !queryParameters.isAdmin && !queryParameters.isTeacher

        property string classCode: ""

        onTriggered: control.tabPage.pushContent(componentQR, {
                                                     serverFunc: "register",
                                                     serverQueries: {
                                                         code: actionQR.classCode,
                                                         server: cosClient.serverUuid,
                                                         oauth2: "0"
                                                     },
                                                     displayText: (actionQR.classCode !== "" ?
                                                                       ("<br>%1: <b>%2</b>").arg(qsTr("Hitelesítő kód")).arg(actionQR.classCode)
                                                                     : "")

                                                 })

    }


    Action {
        id: actionQRgoogle
        icon.source: "qrc:/internal/img/google.svg"
        text: qsTr("Regisztrációs info")
        enabled: !queryParameters.isAdmin && !queryParameters.isTeacher

        onTriggered: control.tabPage.pushContent(componentQR, {
                                                     serverFunc: "register",
                                                     serverQueries: {
                                                         code: actionQR.classCode,
                                                         server: cosClient.serverUuid,
                                                         oauth2: "1"
                                                     },
                                                     displayText: (actionQR.classCode !== "" ?
                                                                       ("<br>%1: <b>%2</b>").arg(qsTr("Hitelesítő kód")).arg(actionQR.classCode)
                                                                     : "")

                                                 })

    }


    Component {
        id: componentQR
        ServerInfo {  }
    }

    backCallbackFunction: function () {
        if (serverSettings.modelUserList.selectedCount) {
            serverSettings.modelUserList.unselectAll()
            return true
        }

        return false
    }

    Component.onCompleted: {
        if (queryParameters.isAdmin === true)
            contextMenu.addAction(actionRevokeAdmin)
        else if (queryParameters.isTeacher === true) {
            contextMenu.addAction(actionToStudent)
            contextMenu.addAction(actionToAdmin)
        } else
            contextMenu.addAction(actionToTeacher)

        _filteredClassModel.clear()

        if (queryParameters.classid !== -1)
            _filteredClassModel.append({id: -1, name: qsTr("[Osztály nélkül]")})

        if (!tabPage)
            return

        for (var i=0; i<tabPage.classModel.length; i++) {
            var d=tabPage.classModel[i]
            if (d.id !== queryParameters.classid)
                _filteredClassModel.append({id: d.id, name: d.name})
        }
    }

}
