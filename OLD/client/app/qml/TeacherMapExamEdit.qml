import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS



QTabContainer {
    id: control

    contentTitle: qsTr("Dolgozat szerkesztése")
    title: qsTr("Dolgozat szerkesztése")
    icon: "qrc:/internal/icon/wrench.svg"
    action: actionSaveExam

    property string uuid: ""
    property TeacherMaps tMaps: null
    property TeacherGroups tGroups: null

    property bool _closeEnabled: false

    ListModel {
        id: modelChapter
    }

    QAccordion {
        id: acc

        QTabHeader {
            tabContainer: control
            flickable: acc.flickable
        }

        QCollapsible {
            title: qsTr("Általános")

            isFullscreen: control.compact

            QGridLayout {
                id: grid1
                enabled: tMaps ? !tMaps.isBusy :
                                 tGroups ? !tGroups.isBusy :
                                           false

                isFullscreen: control.compact

                onModifiedChanged: updateSaveEnabled()

                watchModification: true

                QGridLabel {
                    field: textTitle
                }

                QGridTextField {
                    id: textTitle
                    fieldName: qsTr("Cím")
                    sqlField: "title"

                    validator: RegExpValidator { regExp: /.+/ }
                }

                QGridLabel {
                    field: textDescription
                }

                QGridTextArea {
                    id: textDescription
                    fieldName: qsTr("Leírás")
                    sqlField: "description"
                    minimumHeight: CosStyle.baseHeight*3
                }

                QGridText {
                    text: qsTr("Feladatok száma:")
                    field: spinCount
                }

                QGridSpinBox {
                    id: spinCount
                    from: 1
                    to: 99
                    editable: true
                }

            }
        }

        QCollapsible {
            id: collapsibleGrade

            title: qsTr("Értékelés")

            isFullscreen: control.compact

            property bool modified: false
            property real textWidth: 100

            onModifiedChanged: updateSaveEnabled()

            Column {
                Repeater {
                    id: rptrGrade

                    model: ListModel {
                        id: modelGrade
                    }

                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        QLabel {
                            text: "%1 (%2)".arg(longname).arg(shortname)
                            width: collapsibleGrade.textWidth

                            anchors.verticalCenter: parent.verticalCenter

                            horizontalAlignment: Text.AlignHCenter

                            font.weight: Font.DemiBold

                            Component.onCompleted: collapsibleGrade.textWidth = Math.max(collapsibleGrade.textWidth, implicitWidth)
                        }

                        QSpinBox {
                            from: 0
                            to: 100
                            stepSize: 5
                            editable: true

                            anchors.verticalCenter: parent.verticalCenter

                            value: percent

                            textFromValue: function(value) {
                                return String("%1%").arg(value)
                            }

                            valueFromText: function(text) {
                                return Number(String(text).replace("%", ""))
                            }

                            onValueModified: {
                                modelGrade.setProperty(index, "percent", value)
                                collapsibleGrade.modified = true
                            }
                        }
                    }
                }

            }
        }


        QCollapsible {
            title: qsTr("Szakaszok")

            isFullscreen: control.compact

            rightComponent: QToolButton {
                icon.source: CosStyle.iconAdd
                //icon.width: CosStyle.pixelSize*0.8
                //icon.height: CosStyle.pixelSize*0.8
                ToolTip.text: qsTr("Kijelölés törlése")
                onClicked: {
                    JS.listModelUpdatePropertyAll(modelChapter, "selected", false)
                    listChapter.selectionModified()
                }
            }

            QObjectListView {
                id: listChapter

                property bool modified: false

                width: parent.width

                selectorSet: true

                isFullscreen: control.compact

                model: SortFilterProxyModel {
                    sourceModel: modelChapter

                    sorters: [
                        StringSorter { roleName: "name"; priority: 0 }
                    ]
                }

                modelTitleRole: "name"
                delegateHeight: CosStyle.halfLineHeight

                onSelectionModified: {
                    modified = true
                    updateSaveEnabled()
                }

            }
        }

    }


    Connections {
        target: tMaps

        function onExamGet(jsonData, binaryData) {
            JS.setSqlFields([
                                textTitle,
                                textDescription
                            ], jsonData)

            var c = jsonData.config && jsonData.config !== "" ? JSON.parse(jsonData.config) : {}

            var cl = c ? c.chapters : null

            spinCount.setData(c && c.count ? c.count : 10)

            for (var i=0; i<modelChapter.count; ++i) {
                modelChapter.setProperty(i, "selected", cl ? cl.includes(modelChapter.get(i).id) : false)
            }


            // Grading

            var gl = jsonData.grading && jsonData.grading !== "" ? JSON.parse(jsonData.grading) : []

            modelGrade.clear()

            var n = 0

            if (jsonData.gradeList) {
                for (i=0; i<jsonData.gradeList.length; ++i) {
                    var o = jsonData.gradeList[i]

                    if (gl && gl.length) {
                        for (var j=0; j<gl.length; ++j) {
                            if (gl[j].value === o.value)
                                n = Number(gl[j].percent)
                        }
                    } else if (jsonData.gradeList.length > 1) {
                        n = Math.round(100*i/(jsonData.gradeList.length-1))
                    }

                    o["percent"] = n
                    modelGrade.append(o)
                }
            }

            grid1.modified = false
            listChapter.modified = false
            collapsibleGrade.modified = false
            updateSaveEnabled()
        }

        function onExamAdd(jsonData, binaryData) {
            if (jsonData.created === true && uuid == "") {
                _closeEnabled = true
                mainStack.back()
            }
        }


        function onExamModify(jsonData, binaryData) {
            if (jsonData.modified === true && jsonData.uuid === uuid) {
                grid1.modified = false
                listChapter.modified = false
                collapsibleGrade.modified = false
                updateSaveEnabled()
            }
        }
    }



    onPopulated: {
        if (tMaps)
            tMaps.send("examGet", {uuid: uuid})
        else if (tGroups)
            tGroups.send("examGet", {uuid: uuid})
    }




    Action {
        id: actionSaveExam
        icon.source: CosStyle.iconSave
        //text: qsTr("Mentés")
        enabled: false
        shortcut: "Ctrl+S"

        onTriggered: {
            var o = JS.getSqlFields([
                                        textTitle,
                                        textDescription
                                    ])


            // Config
            var c = {}

            c["chapters"] = JS.listModelGetSelectedFields(modelChapter, "id")
            c["count"] = spinCount.value


            o["config"] = JSON.stringify(c)


            // Grading

            var gl = []

            for (var i=0; i<modelGrade.count; ++i) {
                var g = modelGrade.get(i)
                gl.push({
                            value: g.value,
                            percent: g.percent
                        })
            }

            o["grading"] = JSON.stringify(gl)



            if (uuid == "") {
                if (tMaps) {
                    o["mapuuid"] = tMaps.selectedMapId
                    tMaps.send("examAdd", o)
                }
            } else {
                o["uuid"] = uuid
                if (tMaps) {
                    tMaps.send("examModify", o)
                }
            }
        }
    }


    function updateSaveEnabled() {
        actionSaveExam.enabled = grid1.modified || listChapter.modified || collapsibleGrade.modified
    }


    Component.onCompleted: {
        if (tMaps) {
            JS.listModelReplaceAddSelected(modelChapter, tMaps.getSelectedMapChapters())
        }
    }


    backCallbackFunction: function () {
        if (_closeEnabled)
            return false

        if (actionSaveExam.enabled) {
            var d = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan eldobod a módosításokat?")})
            d.accepted.connect(function() {
                _closeEnabled = true
                mainStack.back()
            })
            d.open()
            return true
        }

        return false
    }


    closeCallbackFunction: function () {
        if (_closeEnabled)
            return false

        if (actionSaveExam.enabled) {
            var d = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan eldobod a módosításokat?")})
            d.accepted.connect(function() {
                _closeEnabled = true
                mainWindow.close()
            })
            d.open()
            return true
        }

        return false
    }
}
