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

    title: qsTr("Hadjárat szerkesztése")
    icon: "qrc:/internal/icon/calendar-edit.svg"
    action: actionSave

    property bool baseModificationEnabled: true

    menu: QMenu {
        MenuItem {
            text: qsTr("Új értékelés")
            icon.source: CosStyle.iconAdd
            enabled: baseModificationEnabled
            onClicked: {
                modelAssignment.append({
                                           id: -1,
                                           name: ""
                                       })
                actionSave.enabled = true
            }
        }

        MenuItem {
            id: menuRemove
            text: qsTr("Hadjárat törlése")
            icon.source: "qrc:/internal/icon/calendar-remove.svg"
            enabled: campaignId != -1
            onClicked: {
                var d = JS.dialogCreateQml("YesNo", {
                                               text: qsTr("Biztosan törlöd a hadjáratot?"),
                                               image: "qrc:/internal/icon/calendar-remove.svg"
                                           })
                d.accepted.connect(function() {
                    actionSave.enabled = false
                    mainStack.back()
                })
                d.open()
                return true
            }
        }
    }

    property int campaignId: -1

    QAccordion {

        QTabHeader {
            tabContainer: control
            isPlaceholder: true
        }

        QCollapsible {
            title: qsTr("Alapadatok")
            backgroundColor: "transparent"

            QGridLayout {
                id: layout1

                watchModification: true
                columns: 2

                onAccepted: save()

                Rectangle {
                    id: rectState

                    Layout.bottomMargin: 10
                    Layout.leftMargin: 30
                    Layout.rightMargin: 30
                    Layout.fillWidth: true
                    Layout.columnSpan: parent.columns

                    color: "transparent"

                    implicitWidth: labelState.implicitWidth
                    implicitHeight: labelState.implicitHeight

                    QLabel {
                        id: labelState
                        anchors.centerIn: parent
                        font.pixelSize: CosStyle.pixelSize*1.2
                        font.weight: Font.DemiBold
                        text: qsTr("Előkészítés alatt")
                        color: CosStyle.colorAccent
                        font.capitalization: Font.AllUppercase
                        padding: 10
                    }

                    states: [
                        State {
                            name: "started"
                            PropertyChanges {
                                target: rectState
                                color: CosStyle.colorOKDarker
                            }
                            PropertyChanges {
                                target: labelState
                                color: CosStyle.colorAccentLight
                                text: qsTr("Folyamatban")
                            }
                        },
                        State {
                            name: "finished"
                            PropertyChanges {
                                target: rectState
                                color: CosStyle.colorWarningDark
                            }
                            PropertyChanges {
                                target: labelState
                                color: "black"
                                text: qsTr("Befejeződött")
                            }
                        }
                    ]
                }

                QGridLabel {
                    field: textDescription
                }

                QGridTextField {
                    id: textDescription
                    fieldName: qsTr("Leírás")
                    sqlField: "description"
                    placeholderText: qsTr("A hadjárat leírása (opcionális)")
                    readOnly: !baseModificationEnabled
                }

                QGridLabel { field: textStarttime }

                QGridTextField {
                    id: textStarttime
                    fieldName: qsTr("Kezdés")
                    sqlField: "starttime"
                    placeholderText: qsTr("Kezdő időpont (YYYY-MM-DD HH:mm)")
                    readOnly: !baseModificationEnabled

                    //validator: RegExpValidator { regExp: /.+/ }
                }

                QGridLabel { field: textEndtime }

                QGridTextField {
                    id: textEndtime
                    fieldName: qsTr("Befejezés")
                    sqlField: "endtime"
                    placeholderText: qsTr("Befejező időpont (YYYY-MM-DD HH:mm)")
                }


                QGridLabel { field: areaMapOpen }

                QGridTextArea {
                    id: areaMapOpen
                    fieldName: qsTr("Pályák nyitása")
                    placeholderText: qsTr("A kezdő időpontban aktiválandó a pályák")
                    minimumHeight: CosStyle.baseHeight*2
                    readOnly: true
                    //color: CosStyle.colorWarning
                }

                QGridButton {
                    id: buttonMapOpen
                    text: qsTr("Kiválasztás")

                    icon.source: "qrc:/internal/icon/briefcase-search.svg"

                    enabled: baseModificationEnabled

                    onClicked: {
                        JS.listModelCopy(_modelDialogList, modelMapOpenList)
                        var d = JS.dialogCreateQml("List", {
                                                       icon: buttonMapOpen.icon.source,
                                                       title: qsTr("Pályák nyitása"),
                                                       selectorSet: true,
                                                       modelTitleRole: "name",
                                                       model: _modelDialogList
                                                   })


                        d.accepted.connect(function(data) {
                            if (!data)
                                return

                            JS.listModelCopy(modelMapOpenList, _modelDialogList)
                            updateMapArea(areaMapOpen, modelMapOpenList)

                            //studentMaps.send("campaignGet", {groupid: studentMaps.selectedGroupId, id: data.id})*/
                        })
                        d.open()
                    }

                }


                QGridLabel { field: areaMapClose }

                QGridTextArea {
                    id: areaMapClose
                    fieldName: qsTr("Pályák zárása")
                    placeholderText: qsTr("A bejező időpontban inaktiválandó a pályák")
                    minimumHeight: CosStyle.baseHeight*2
                    readOnly: true
                    //color: CosStyle.colorWarning
                }

                QGridButton {
                    id: buttonMapClose
                    text: qsTr("Kiválasztás")

                    icon.source: "qrc:/internal/icon/briefcase-search.svg"

                    onClicked: {
                        JS.listModelCopy(_modelDialogList, modelMapCloseList)
                        var d = JS.dialogCreateQml("List", {
                                                       icon: buttonMapClose.icon.source,
                                                       title: qsTr("Pályák zárása"),
                                                       selectorSet: true,
                                                       modelTitleRole: "name",
                                                       model: _modelDialogList
                                                   })


                        d.accepted.connect(function(data) {
                            if (!data)
                                return

                            JS.listModelCopy(modelMapCloseList, _modelDialogList)
                            updateMapArea(areaMapClose, modelMapCloseList)

                            //studentMaps.send("campaignGet", {groupid: studentMaps.selectedGroupId, id: data.id})*/
                        })
                        d.open()
                    }

                    /*onClicked: {
                        if (server) {
                            JS.updateByModifiedSqlFields(server, [textHostname, spinPort, checkSsl])
                        } else {
                            var m = JS.getSqlFields([textHostname, spinPort, checkSsl])

                            if (Object.keys(m).length) {
                                servers.serverCreate(m)
                            }
                        }
                        servers.uiBack()
                    }*/

                }


            }
        }


        Repeater {
            model: ListModel {
                id: modelAssignment
            }

            QCollapsible {
                title: qsTr("Értékelés #%1").arg(index+1)
                required property int index
                required property string name

                rightComponent: QToolButton {
                    icon.source: "qrc:/internal/icon/delete.svg"
                    color: CosStyle.colorErrorLighter
                    ToolTip.text: qsTr("Értékelés törlése")
                    enabled: baseModificationEnabled
                    onClicked: {
                        var d = JS.dialogCreateQml("YesNo", {
                                                       text: qsTr("Biztosan törlöd az értékelést?"),
                                                       image: "qrc:/internal/icon/delete.svg"
                                                   })
                        d.accepted.connect(function() {
                            actionSave.enabled = true
                            modelAssignment.remove(index)
                        })
                        d.open()
                    }
                }

                QLabel {
                    text: "szia "+name
                }
            }

        }


        QButton {
            id: buttonAdd
            text: qsTr("Hadjárat létrehozása")
            icon.source: "qrc:/internal/icon/content-save.svg"
            visible: campaignId == -1
            themeColors: CosStyle.buttonThemeGreen
            anchors.horizontalCenter: parent.horizontalCenter
            padding: 20
        }
    }



    Connections {
        target: teacherGroups

        function onCampaignGetReady(jsonData) {

            if (jsonData.finished === true) {
                baseModificationEnabled = false
                textEndtime.readOnly = true
                buttonMapClose.enabled = false
            } else if (jsonData.started === true) {
                baseModificationEnabled = false
                textEndtime.readOnly = false
                buttonMapClose.enabled = true
            } else {
                baseModificationEnabled = true
                textEndtime.readOnly = false
                buttonMapClose.enabled = true
            }

            JS.setSqlFields([
                                textDescription,
                                textStarttime,
                                textEndtime
                            ], jsonData)


            // Map models

            modelMapOpenList.clear()
            modelMapCloseList.clear()

            for (var i=0; i<teacherGroups.mapList.length; i++) {
                var d = teacherGroups.mapList[i]
                d.selected = false
                modelMapOpenList.append(d)
                modelMapCloseList.append(d)
            }


            // Default assignment

            if (jsonData.assignmentList === undefined || jsonData.assignmentList.length === 0) {
                modelAssignment.clear()
                modelAssignment.append({
                                           id: -1,
                                           name: ""
                                       })
            }


            // RETURN IF NEW

            if (jsonData.error !== undefined)
                return


            if (jsonData.started === true || jsonData.finished === true)
                menuRemove.enabled = false

            if (jsonData.finished === true)
                rectState.state = "finished"
            else if (jsonData.started === true)
                rectState.state = "started"

            // Map Open text area

            var t1 = ""

            for (i=0; i<jsonData.mapopen.length; i++) {
                var uuid = jsonData.mapopen[i]

                for (var j=0; j<modelMapOpenList.count; j++) {
                    var o = modelMapOpenList.get(j)
                    if (o.uuid === uuid) {
                        modelMapOpenList.setProperty(j, "selected", true)
                        uuid = o.name
                    }
                }

                if (t1 == "")
                    t1 = uuid
                else
                    t1 += "\n"+uuid
            }

            areaMapOpen.text = t1


            // Map Close text area

            t1 = ""

            for (i=0; i<jsonData.mapclose.length; i++) {
                uuid = jsonData.mapclose[i]

                for (j=0; j<modelMapCloseList.count; j++) {
                    o = modelMapCloseList.get(j)
                    if (o.uuid === uuid) {
                        modelMapCloseList.setProperty(j, "selected", true)
                        uuid = o.name
                    }
                }

                if (t1 == "")
                    t1 = uuid
                else
                    t1 += "\n"+uuid
            }

            areaMapClose.text = t1


            // Assignments

            JS.listModelReplace(modelAssignment, jsonData.assignmentList)

        }
    }

    onPopulated: {
        teacherGroups.send("campaignGet", {id: campaignId})
    }



    Action {
        id: actionSave
        icon.source: "qrc:/internal/icon/content-save.svg"
        shortcut: "Ctrl+S"
        enabled: campaignId == -1 || layout1.modified
        onTriggered: save()
    }

    ListModel {
        id: modelMapOpenList
    }

    ListModel {
        id: modelMapCloseList
    }

    ListModel {
        id: _modelDialogList
    }


    function updateMapArea(area, model) {
        var t1 = ""
        for (var i=0; i<model.count; i++) {
            var n = model.get(i)

            if (n.selected !== true)
                continue

            if (t1 == "")
                t1 = n.name
            else
                t1 += "\n"+n.name
        }

        area.text = t1
        area.modified = true
        area.parent.modified = true
    }

    function save() {
        /*var o = JS.getModifiedSqlFields([
                                            textFirstname,
                                            textLastname,
                                            textNickname,
                                            spinCharacter
                                        ])

        if (username === cosClient.userName && Object.keys(o).length) {
            if (cosClient.userRoles & Client.RoleTeacher)
                profile.send(CosMessage.ClassTeacher, "userModify", o)
            else
                profile.send(CosMessage.ClassStudent, "userModify", o)
        }*/

        layout1.modified = false
    }


    backCallbackFunction: function () {
        if (actionSave.enabled) {
            var d = JS.dialogCreateQml("YesNo", {
                                           text: qsTr("Biztosan elveted a módosításokat?"),
                                           image: "qrc:/internal/icon/close-octagon-outline.svg"
                                       })
            d.accepted.connect(function() {
                actionSave.enabled = false
                mainStack.back()
            })
            d.open()
            return true
        }

        return false
    }
}



