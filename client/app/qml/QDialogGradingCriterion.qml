import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import COS.Client 1.0
import "Style"
import "JScript.js" as JS
import "."

QDialogPanel {
    id: item

    title: qsTr("Kritérium")

    icon: "qrc:/internal/icon/message-cog-outline.svg"

    property TeacherGroups teacherGroups: null
    property var grading: ({})


    QGridLayoutFlickable {
        id: grid

        implicitWidth: item.panel.width-10

        watchModification: false

        onAccepted: buttonYes.press()

        QGridText {
            text: qsTr("Típus")
        }

        QGridComboBox {
            id: comboType
            sqlField: "type"

            valueRole: "value"
            textRole: "text"

            model: [
                {value: "grade", text: qsTr("Jegy")},
                {value: "xp", text: qsTr("XP")},
            ]
        }

        QGridText {
            text: qsTr("Jegy")
            visible: comboGrade.visible
        }

        QGridComboBox {
            id: comboGrade
            sqlField: "ref"

            visible: comboType.currentValue === "grade"

            valueRole: "id"
            textRole: "longname"

            model: ListModel {}
        }


        QGridText {
            text: qsTr("XP")
            visible: spinXP.visible
        }

        QGridSpinBox {
            id: spinXP
            sqlField: "value"

            from: 10
            to: 1000
            editable: true

            visible: comboType.currentValue === "xp"
        }


        QGridText {
            text: qsTr("Mód")
        }

        QGridComboBox {
            id: comboMode
            sqlField: "mode"

            valueRole: "value"
            textRole: "text"

            model: [
                {value: "", text: qsTr("Normál")},
                {value: "default", text: qsTr("Alapértelmezett")},
                {value: "required", text: qsTr("Minimum követelmény")},
            ]
        }


        QGridText {
            text: qsTr("Kritérium")
        }

        QGridComboBox {
            id: comboModule
            sqlField: "module"

            valueRole: "value"
            textRole: "text"

            model: [
                {value: "", text: qsTr("--- Nincs ---")},
                {value: "xp", text: qsTr("XP összegyűjtése")},
                {value: "trophy", text: qsTr("Trófea összegyűjtése")},
                {value: "sumlevel", text: qsTr("Adott szintek teljesítése")},
                {value: "missionlevel", text: qsTr("Adott küldetés teljesítése")}
            ]
        }

        QGridText {
            text: qsTr("Pálya")
            visible: comboMap.visible
        }

        QGridComboBox {
            id: comboMap
            sqlField: "map"

            valueRole: "uuid"
            textRole: "name"

            visible: comboModule.currentValue === "missionlevel"

            model: ListModel {}

            onCurrentValueChanged: {
                if (teacherGroups && currentIndex != -1)
                    JS.listModelReplace(comboMission.model, teacherGroups.missionList(currentValue))
                else
                    comboMission.model.clear()
                comboMission.recalculate()
            }
        }


        QGridText {
            text: qsTr("Küldetés")
            visible: comboMission.visible
        }

        QGridComboBox {
            id: comboMission
            sqlField: "mission"

            valueRole: "uuid"
            textRole: "name"

            visible: comboModule.currentValue === "missionlevel"

            model: ListModel {}
        }

        QGridText {
            text: qsTr("Szint")
            visible: comboLevel.visible
        }

        QGridComboBox {
            id: comboLevel
            sqlField: "level"

            valueRole: "value"
            textRole: "text"

            visible: comboModule.currentValue === "missionlevel" || comboModule.currentValue === "sumlevel"

            model: [
                {value: "t1", text: qsTr("Level 1")},
                {value: "d1", text: qsTr("Level 1 sudden death")},
                {value: "t2", text: qsTr("Level 2")},
                {value: "d2", text: qsTr("Level 2 sudden death")},
                {value: "t3", text: qsTr("Level 3")},
                {value: "d3", text: qsTr("Level 3 sudden death")}
            ]
        }

        QGridText {
            text: qsTr("Darab")
            visible: spinValue.visible
        }

        QGridSpinBox {
            id: spinValue
            sqlField: "value"

            from: 1
            to: 10000
            editable: true

            visible: comboModule.currentValue === "xp" || comboModule.currentValue === "trophy" || comboModule.currentValue === "sumlevel"
        }


        QGridText {
            Layout.alignment: Qt.AlignCenter
            Layout.columnSpan: parent.columns
            Layout.fillWidth: true
            Layout.topMargin: 10
            Layout.bottomMargin: 10
            horizontalAlignment: Text.AlignHCenter

            color: CosStyle.colorErrorLighter
            font.bold: Font.DemiBold

            text: buttonYes.enabled ? "" : qsTr("Minden mezőt ki kell tölteni!")

        }


    }

    buttons: Row {
        id: buttonRow
        spacing: 10

        anchors.horizontalCenter: parent.horizontalCenter

        QButton {
            id: buttonNo
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Mégsem")
            icon.source: "qrc:/internal/icon/close-circle.svg"
            themeColors: CosStyle.buttonThemeRed

            onClicked: {
                dlgClose()
            }
        }

        QButton {
            id: buttonYes

            anchors.verticalCenter: parent.verticalCenter

            text: qsTr("OK")
            icon.source: "qrc:/internal/icon/check-bold.svg"
            themeColors: CosStyle.buttonThemeGreen

            onClicked: {
                var d = {}

                d.type = comboType.currentValue

                if (d.type === "grade") {
                    d.ref = comboGrade.currentValue
                    d.value = -1
                } else if (d.type === "xp") {
                    d.ref = -1
                    d.value = spinXP.value
                }

                var c = {}

                if (comboMode.currentValue !== "")
                    c.mode = comboMode.currentValue

                if (comboModule.currentValue !== "")
                    c.module = comboModule.currentValue

                if (comboModule.currentValue === "xp" || comboModule.currentValue === "trophy" || comboModule.currentValue === "sumlevel")
                    c.value = spinValue.value

                if (comboModule.currentValue === "missionlevel") {
                    c.map = comboMap.currentValue
                    c.mission = comboMission.currentValue
                }

                if (comboModule.currentValue === "missionlevel" || comboModule.currentValue === "sumlevel") {
                    var l = comboLevel.currentValue

                    if (l === "d1" || l === "d2" || l === "d3")
                        c.deathmatch = true
                    else
                        c.deathmatch = false

                    if (l === "t1" || l === "d1")
                        c.level = 1
                    else if (l === "t2" || l === "d2")
                        c.level = 2
                    else if (l === "t3" || l === "d3")
                        c.level = 3
                }

                d.criteria = c
                acceptedData = d

                dlgClose()
            }

            enabled: comboType.currentIndex == -1 ||
                     (comboType.currentValue === "grade" && comboGrade.currentIndex == -1) ||
                     comboMode.currentIndex == -1 ||
                     comboModule.currentIndex == -1 ||
                     (comboModule.currentValue === "missionlevel" && (comboMap.currentIndex == -1 || comboMission.currentIndex == -1 ||
                                                                      comboLevel.currentIndex == -1)) ||
                     (comboModule.currentValue === "sumlevel" && comboLevel.currentIndex == -1) ?
                         false : true
        }
    }


    function populated() {
        buttonNo.forceActiveFocus()
    }

    Component.onCompleted: {
        if (!teacherGroups)
            return

        JS.listModelReplace(comboGrade.model, teacherGroups.gradeList)
        JS.listModelReplace(comboMap.model, teacherGroups.mapList)

        comboGrade.recalculate()
        comboMap.recalculate()

        comboType.setData(grading.type)

        if (grading.type === "grade")
            comboGrade.setData(grading.ref)

        if (grading.type === "xp")
            spinXP.setData(grading.value)

        if (grading.criteria !== undefined) {
            var c = grading.criteria
            comboMode.setData(c.mode)
            comboModule.setData(c.module)

            if (c.map !== undefined && c.mission !== undefined) {
                comboMap.setData(c.map)
                comboMission.setData(c.mission)
            }

            if (c.value !== undefined)
                spinValue.setData(c.value)

            if (c.level !== undefined) {
                var m = "t1"

                if (c.level === 1)
                    m = (c.deathmatch === true ? "d1" : "t1")
                else if (c.level === 2)
                    m = (c.deathmatch === true ? "d2" : "t2")
                else if (c.level === 3)
                    m = (c.deathmatch === true ? "d3" : "t3")

                comboLevel.setData(m)
            }
        }

    }

}
