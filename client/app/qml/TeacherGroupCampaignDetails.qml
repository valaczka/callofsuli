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

    property int campaignId: -1
    property bool isEditing: campaignId == -1
    property bool baseModificationEnabled: true

    readonly property string _dateFormat: "yyyy-MM-dd HH:mm"

    menu: QMenu {
        MenuItem {
            text: qsTr("Új értékelés")
            icon.source: CosStyle.iconAdd
            enabled: isEditing && baseModificationEnabled
            onClicked: {
                modelAssignment.append({
                                           id: -1,
                                           name: "",
                                           gradingList: []
                                       })
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
                    teacherGroups.send("campaignRemove", {id: campaignId})
                })
                d.open()
                return true
            }
        }
    }


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
                    readOnly: !isEditing || !baseModificationEnabled
                }

                QGridLabel { field: textStarttime }

                QGridTextField {
                    id: textStarttime
                    fieldName: qsTr("Kezdés")
                    sqlField: "starttime"
                    placeholderText: qsTr("Kezdő időpont (%1)").arg(_dateFormat)
                    readOnly: !isEditing || !baseModificationEnabled

                    onTextModified: parseDates()

                    property bool _parseOK: true

                    Binding on textColor {
                        when: !textStarttime._parseOK
                        value: CosStyle.colorErrorLighter
                    }

                    function setData(t) {
                        text = Date.fromLocaleString(Qt.locale(), t, "yyyy-MM-dd HH:mm:ss").toLocaleString(Qt.locale(), _dateFormat)
                        modified = false
                    }
                }

                QGridLabel { field: textEndtime }

                QGridTextField {
                    id: textEndtime
                    fieldName: qsTr("Befejezés")
                    sqlField: "endtime"
                    placeholderText: qsTr("Befejező időpont (%1)").arg(_dateFormat)
                    readOnly: !isEditing

                    onTextModified: parseDates()

                    property bool _parseOK: true

                    Binding on textColor {
                        when: !textEndtime._parseOK
                        value: CosStyle.colorErrorLighter
                    }

                    function setData(t) {
                        text = Date.fromLocaleString(Qt.locale(), t, "yyyy-MM-dd HH:mm:ss").toLocaleString(Qt.locale(), _dateFormat)
                        modified = false
                    }
                }


                QGridLabel { field: areaMapOpen }

                QGridTextArea {
                    id: areaMapOpen
                    fieldName: qsTr("Pályák nyitása")
                    placeholderText: qsTr("A kezdő időpontban aktiválandó pályák")
                    minimumHeight: CosStyle.baseHeight*2
                    readOnly: true
                    //color: CosStyle.colorWarning
                }

                QGridButton {
                    id: buttonMapOpen
                    text: qsTr("Kiválasztás")

                    icon.source: "qrc:/internal/icon/briefcase-search.svg"

                    enabled: isEditing && baseModificationEnabled

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
                        })
                        d.open()
                    }

                }


                QGridLabel { field: areaMapClose }

                QGridTextArea {
                    id: areaMapClose
                    fieldName: qsTr("Pályák zárása")
                    placeholderText: qsTr("A bejező időpontban inaktiválandó pályák")
                    minimumHeight: CosStyle.baseHeight*2
                    readOnly: true
                }

                QGridButton {
                    id: buttonMapClose
                    text: qsTr("Kiválasztás")

                    icon.source: "qrc:/internal/icon/briefcase-search.svg"
                    enabled: isEditing

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
                        })
                        d.open()
                    }

                }


            }
        }


        Repeater {
            model: ListModel {
                id: modelAssignment
            }

            QCollapsible {
                id: assignmentCollapsible
                title: id > 0 ? qsTr("Értékelés #%1").arg(id) : qsTr("*Értékelés")
                required property int index
                required property string name
                required property int id
                required property var gradingList

                rightComponent: QToolButton {
                    icon.source: "qrc:/internal/icon/delete.svg"
                    color: CosStyle.colorErrorLighter
                    ToolTip.text: qsTr("Értékelés törlése")
                    enabled: isEditing && baseModificationEnabled
                    onClicked: {
                        var d = JS.dialogCreateQml("YesNo", {
                                                       text: qsTr("Biztosan törlöd az értékelést?"),
                                                       image: "qrc:/internal/icon/delete.svg"
                                                   })
                        d.accepted.connect(function() {
                            modelAssignment.remove(index)
                        })
                        d.open()
                    }
                }

                Column {
                    width: parent.width

                    Repeater {
                        model: gradingList

                        Item {
                            id: gradingItem

                            required property int index
                            required property int id
                            required property string type
                            required property int ref
                            required property int value
                            required property var criteria

                            width: assignmentCollapsible.width
                            height: CosStyle.pixelSize*4

                            QRectangleBg {
                                anchors.fill: parent
                                acceptedButtons: isEditing && baseModificationEnabled ? Qt.LeftButton : Qt.NoButton

                                QLabel {
                                    id: valueLabel
                                    anchors.left: parent.left
                                    anchors.margins: 5
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: (type == "grade") ? teacherGroups.grade(ref).shortname :
                                                              (type == "xp") ? qsTr("%1 XP").arg(value) :
                                                                               "???"
                                    font.capitalization: Font.AllUppercase
                                    font.pixelSize: CosStyle.pixelSize*2.3
                                    font.weight: Font.Light
                                    color: CosStyle.colorAccentLight
                                    width: CosStyle.pixelSize*8
                                    horizontalAlignment: Text.AlignHCenter
                                }

                                Column {
                                    anchors.left: valueLabel.right
                                    anchors.leftMargin: 10
                                    anchors.rightMargin: 10
                                    anchors.right: btnDeleteCriteria.left
                                    anchors.verticalCenter: parent.verticalCenter

                                    QLabel {
                                        id: modeLabel
                                        anchors.left: parent.left
                                        font.weight: Font.Bold
                                        font.pixelSize: CosStyle.pixelSize*0.8
                                        font.capitalization: Font.AllUppercase
                                        states: [
                                            State {
                                                when: criteria && criteria.mode !== undefined && criteria.mode === "default"
                                                PropertyChanges {
                                                    target: modeLabel
                                                    text: qsTr("Alapértelmezett")
                                                    color: CosStyle.colorOKLighter
                                                }
                                            },
                                            State {
                                                when: criteria && criteria.mode !== undefined && criteria.mode === "required"
                                                PropertyChanges {
                                                    target: modeLabel
                                                    text: qsTr("Minimum követelmény")
                                                    color: CosStyle.colorErrorLighter
                                                }
                                            }
                                        ]
                                    }

                                    QLabel {
                                        id: criteriaLabel
                                        width: parent.width
                                        anchors.left: parent.left
                                        wrapMode: Text.Wrap
                                        visible: text != ""
                                    }
                                }

                                QToolButton {
                                    id: btnDeleteCriteria
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.right: parent.right
                                    icon.source: "qrc:/internal/icon/delete.svg"

                                    color: CosStyle.colorErrorLighter
                                    ToolTip.text: qsTr("Kritérium törlése")
                                    enabled: isEditing && baseModificationEnabled
                                    onClicked: {
                                        var d = JS.dialogCreateQml("YesNo", {
                                                                       text: qsTr("Biztosan törlöd a kritériumot?"),
                                                                       image: "qrc:/internal/icon/delete.svg"
                                                                   })
                                        d.accepted.connect(function() {
                                            gradingList.remove(gradingItem.index)
                                        })
                                        d.open()
                                    }
                                }

                                mouseArea.onClicked: {
                                    var d = JS.dialogCreateQml("GradingCriterion", {
                                                                   teacherGroups: teacherGroups,
                                                                   grading: {
                                                                       id: gradingItem.id,
                                                                       type: gradingItem.type,
                                                                       ref: gradingItem.ref,
                                                                       value: gradingItem.value,
                                                                       criteria: gradingItem.criteria
                                                                   }
                                                               })
                                    d.accepted.connect(function(data) {
                                        if (!data)
                                            return

                                        gradingList.set(gradingItem.index, {
                                                            id: gradingItem.id,
                                                            type: data.type,
                                                            ref: data.ref,
                                                            value: data.value,
                                                            criteria: data.criteria
                                                        })

                                    })
                                    d.open()
                                }
                            }

                            onCriteriaChanged: {
                                if (criteria.module === undefined)
                                    return

                                var m = criteria.module

                                if (m === "xp")
                                    criteriaLabel.text = qsTr("%1 XP összegyűjtése").arg(criteria.value)
                                else if (m === "trophy")
                                    criteriaLabel.text = qsTr("%1 trófea összegyűjtése").arg(criteria.value)
                                else if (m === "sumlevel")
                                    criteriaLabel.text = qsTr("%1 különböző LEVEL %2%3 szintű küldetés").arg(criteria.value).arg(criteria.level).arg(
                                                criteria.deathmatch ? qsTr(" SUDDEN DEATH") : "")
                                else if (m === "missionlevel") {
                                    var mm = teacherGroups.mapMission(criteria.map, criteria.mission)
                                    criteriaLabel.text = qsTr("%1 pálya %2 küldetés LEVEL %3%4 szinten").arg(
                                                mm.map).arg(mm.mission).arg(criteria.level).arg(criteria.deathmatch ? qsTr(" SUDDEN DEATH") : "")
                                } else
                                    criteriaLabel.text = "%1 value: %2".arg(m).arg(criteria.value)
                            }
                        }
                    }

                    QToolButtonFooter {
                        width: parent.width
                        icon.source: CosStyle.iconAdd
                        text: qsTr("Új kritérium")
                        visible: isEditing && baseModificationEnabled
                        onClicked: {
                            var d = JS.dialogCreateQml("GradingCriterion", {
                                                           teacherGroups: teacherGroups,
                                                           grading: {
                                                               id: -1,
                                                               type: "grade",
                                                               value: -1,
                                                               criteria: {}
                                                           }
                                                       })
                            d.accepted.connect(function(data) {
                                if (!data)
                                    return

                                gradingList.append({
                                                       id: -1,
                                                       type: data.type,
                                                       ref: data.ref,
                                                       value: data.value,
                                                       criteria: data.criteria
                                                   })

                            })
                            d.open()
                        }
                    }
                }

            }

        }


        QButton {
            id: buttonAdd
            text: qsTr("Hadjárat létrehozása")
            icon.source: "qrc:/internal/icon/content-save.svg"
            visible: campaignId == -1 && isEditing
            themeColors: CosStyle.buttonThemeGreen
            anchors.horizontalCenter: parent.horizontalCenter
            padding: 20

            onClicked: save()
        }
    }



    Connections {
        target: teacherGroups

        function onCampaignAdd(jsonData, binaryData) {
            if (jsonData.error !== undefined) {
                cosClient.sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg", qsTr("Hiba"), jsonData.error)
                return
            }

            if (campaignId == -1) {
                campaignId = jsonData.id
                teacherGroups.send("campaignGet", {id: campaignId})
            }
        }


        function onCampaignRemove(jsonData, binaryData) {
            if (jsonData.error !== undefined) {
                cosClient.sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg", qsTr("Hiba"), jsonData.error)
                return
            }

            if (campaignId === jsonData.id) {
                isEditing = false
                mainStack.back()
            }
        }


        function onCampaignModify(jsonData, binaryData) {
            if (jsonData.error !== undefined) {
                cosClient.sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg", qsTr("Hiba"), jsonData.error)
                return
            }

            if (campaignId === jsonData.id) {
                teacherGroups.send("campaignGet", {id: campaignId})
            }
        }


        function onCampaignGetReady(jsonData) {
            modelAssignment.clear()

            if (campaignId == -1)
                isEditing = true
            else
                isEditing = false

            if (jsonData.finished === true) {
                baseModificationEnabled = false
                isEditing = false
                actionSave.enabled = false
            } else if (jsonData.started === true) {
                baseModificationEnabled = false
            } else {
                baseModificationEnabled = true
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
                modelAssignment.append({
                                           id: -1,
                                           name: "",
                                           gradingList: []
                                       })
            }


            // RETURN IF NEW

            if (jsonData.error !== undefined) {
                areaMapOpen.text = ""
                areaMapClose.text = ""
                return
            }


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
        icon.source: isEditing ? "qrc:/internal/icon/content-save.svg" : "qrc:/internal/icon/pencil.svg"
        onTriggered: {
            if (isEditing) {
                save()
            } else {
                isEditing = true
            }
        }
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
        var p = parseDates()

        if (p !== "") {
            var err = qsTr("Érvénytelen dátum!")

            if (p === "invalid1")
                err = qsTr("Érvénytelen kezdő időpont!")
            else if (p === "invalid2")
                err = qsTr("Érvénytelen befejező időpont!")
            else if (p === "old1")
                err = qsTr("A kezdő időpont a múltban van!")
            else if (p === "old2")
                err = qsTr("A befejező időpont a múltban van!")
            else if (p === "before")
                err = qsTr("A befejező időpont a kezdő időpont előtt van!")

            cosClient.sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", qsTr("Hadjárat szerkesztése"), err)

            return
        }

        var o = {}

        if (textDescription.modified)
            o.description = textDescription.text

        if (textStarttime.modified)
            o.starttime = Date.fromLocaleString(Qt.locale(), textStarttime.text, _dateFormat).toLocaleString(Qt.locale(), "yyyy-MM-dd HH:mm:ss")

        if (textEndtime.modified)
            o.endtime = Date.fromLocaleString(Qt.locale(), textEndtime.text, _dateFormat).toLocaleString(Qt.locale(), "yyyy-MM-dd HH:mm:ss")

        if (areaMapOpen.modified)
            o.mapopen = JS.listModelGetSelectedFields(modelMapOpenList, "uuid")

        if (areaMapClose.modified)
            o.mapclose = JS.listModelGetSelectedFields(modelMapCloseList, "uuid")

        var alist = []

        for (var i=0; i<modelAssignment.count; i++) {
            var a = modelAssignment.get(i)

            alist.push(a)
        }

        o.assignmentList = alist

        if (campaignId == -1) {
            o.groupid = teacherGroups.selectedGroupId
            teacherGroups.send("campaignAdd", o)
        } else {
            o.id = campaignId
            teacherGroups.send("campaignModify", o)
        }
    }


    function parseDates() {
        if (!isEditing)
            return

        var d1 = Date.fromLocaleString(Qt.locale(), textStarttime.text, _dateFormat)
        var d2 = Date.fromLocaleString(Qt.locale(), textEndtime.text, _dateFormat)

        var t1 = d1.getTime()
        var t2 = d2.getTime()

        if (isNaN(t1)) {
            textStarttime._parseOK = false
            return "invalid1"
        }

        if (isNaN(t2)) {
            textEndtime._parseOK = false
            return "invalid2"
        }

        if (t1 <= Date.now() && baseModificationEnabled) {
            textStarttime._parseOK = false
            return "old1"
        }

        textStarttime._parseOK = true

        if (t2 <= Date.now()) {
            textEndtime._parseOK = false
            return "old2"
        }

        if (t2 <= t1) {
            textEndtime._parseOK = false
            return "before"
        }

        textEndtime._parseOK = true

        return ""
    }


    backCallbackFunction: function () {
        if (isEditing && campaignId != -1) {
            teacherGroups.send("campaignGet", {id: campaignId})
            return true
        } if (isEditing && layout1.modified) {
            var d = JS.dialogCreateQml("YesNo", {
                                           text: qsTr("Biztosan elveted a módosításokat?"),
                                           image: "qrc:/internal/icon/close-octagon-outline.svg"
                                       })
            d.accepted.connect(function() {
                isEditing = false
                mainStack.back()
            })
            d.open()
            return true
        }

        return false
    }
}



