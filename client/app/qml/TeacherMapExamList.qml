import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
    id: control

    title: contentTitle
    contentTitle: qsTr("Dolgozatok")
    icon: CosStyle.iconPackage

    ListModel {
        id: modelExam
    }

    QObjectListView {
        id: list

        anchors.fill: parent

        isFullscreen: control.compact

        model: SortFilterProxyModel {
            sourceModel: modelExam

            sorters: [
                StringSorter { roleName: "title"; priority: 0 }
            ]
        }

        modelTitleRole: "title"
        modelSubtitleRole: "description"

        autoSelectorChange: true

        refreshEnabled: true

        delegateHeight: CosStyle.twoLineHeight

        header: QTabHeader {
            tabContainer: control
            flickable: list
        }


        footer: QToolButtonFooter {
            width: list.width
            action: actionExamAdd
        }


        onRefreshRequest: if (teacherMaps.selectedMapId != "")
                              teacherMaps.send("examListGet", {mapuuid: teacherMaps.selectedMapId})

        onClicked: {
            var o = list.modelObject(index)

            control.tabPage.pushContent(cmpEdit, {
                                            uuid: o.uuid
                                        })

        }

        onRightClicked: contextMenu.popup()

        onLongPressed: {
            if (selectorSet) {
                contextMenu.popup()
                return
            }
        }



        QMenu {
            id: contextMenu

            MenuItem { action: actionExamAdd }
            MenuItem { action: actionRemove }
        }


        onKeyInsertPressed: actionExamAdd.trigger()
        onKeyDeletePressed: actionRemove.trigger()
    }



    QIconEmpty {
        visible: modelExam.count === 0
        anchors.centerIn: parent
        textWidth: parent.width*0.75
        text: qsTr("Egyetlen dolgozat sem tartozik még ehhez a pályához")
        tabContainer: control
    }



    Action {
        id: actionExamAdd
        text: qsTr("Új dolgozat")
        icon.source: "qrc:/internal/icon/briefcase-plus.svg"
        enabled: teacherMaps.selectedMapId != ""
        onTriggered: {
            control.tabPage.pushContent(cmpEdit, {
                                            uuid: ""
                                        })
        }
    }



    Action {
        id: actionRemove
        text: qsTr("Törlés")
        icon.source: "qrc:/internal/icon/briefcase-minus.svg"
        enabled: !teacherMaps.isBusy && list.currentIndex !== -1
        onTriggered: {
            var o = list.modelObject(list.currentIndex)

            var more = JS.listModelGetSelectedFields(modelExam, "uuid")

            if (more.length > 0) {
                var dd = JS.dialogCreateQml("YesNo", {
                                                title: qsTr("Dolgozat törlése"),
                                                text: qsTr("Biztosan törlöd a kijelölt %1 dolgozatot?").arg(more.length),
                                                image: "qrc:/internal/icon/briefcase-minus.svg"
                                            })
                dd.accepted.connect(function () {
                    teacherMaps.send("examRemove", { list: more })
                })
                dd.open()
            } else {
                var d = JS.dialogCreateQml("YesNo", {
                                               title: qsTr("Dolgozat törlése"),
                                               text: qsTr("Biztosan törlöd a dolgozatot?\n%1").arg(o.title),
                                               image: "qrc:/internal/icon/briefcase-minus.svg"
                                           })
                d.accepted.connect(function () {
                    teacherMaps.send("examRemove", { uuid: o.uuid })
                })
                d.open()
            }
        }
    }


    Component {
        id: cmpEdit

        TeacherMapExamEdit {
            tMaps: teacherMaps
        }
    }



    Connections {
        target: teacherMaps

        function onExamListGet(jsonData, binaryData) {
            if (jsonData.mapuuid !== teacherMaps.selectedMapId) {
                console.warn(qsTr("Invalid map uuid"), jsonData.mapuuid)
                return
            }

            JS.listModelReplaceAddSelected(modelExam, jsonData.list)

        }

        function onExamAdd(jsonData, binaryData) {
            if (teacherMaps.selectedMapId != "")
                teacherMaps.send("examListGet", {mapuuid: teacherMaps.selectedMapId})
        }

        function onExamRemove(jsonData, binaryData) {
            if (teacherMaps.selectedMapId != "")
                teacherMaps.send("examListGet", {mapuuid: teacherMaps.selectedMapId})
        }

        function onExamModify(jsonData, binaryData) {
            if (teacherMaps.selectedMapId != "")
                teacherMaps.send("examListGet", {mapuuid: teacherMaps.selectedMapId})
        }
    }



    onPopulated: {
        list.forceActiveFocus()

        if (teacherMaps.selectedMapId != "")
            teacherMaps.send("examListGet", {mapuuid: teacherMaps.selectedMapId})
    }

}



