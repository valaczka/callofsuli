import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QTabPage {
    id: control

    property alias groupId: teacherGroups.selectedGroupId
    property alias allGroupList: teacherGroups.allGroupList

    title: teacherGroups.selectedGroupFullName

    //buttonColor: CosStyle.colorPrimary
    buttonBackgroundColor: Qt.darker("#006400")

    buttonModel: ListModel {
        id: modelGuest

        ListElement {
            title: qsTr("Résztvevők")
            icon: "image://font/School/\uf154"
            iconColor: "orchid"
            func: function() { replaceContent(cmpTeacherGroupUserList) }
            checked: true
        }
        ListElement {
            title: qsTr("Pályák")
            icon: "image://font/Academic/\uf122"
            iconColor: "lime"
            func: function() { replaceContent(cmpTeacherGroupMapList) }
        }
        ListElement {
            title: qsTr("Hadjáratok")
            icon: "qrc:/internal/icon/calendar-multiple.svg"
            iconColor: "tomato"
            func: function() { replaceContent(cmpTeacherCampaign) }
        }
        ListElement {
            title: qsTr("Dolgozatok")
            icon: "image://font/AcademicI/\uf15d"
            iconColor: "tomato"
            func: function() { replaceContent(cmpTeacherGroupExamList) }
        }
    }


    activity: TeacherGroups {
        id: teacherGroups

        property var allGroupList: []

        onGroupRemove: {
            if (jsonData.error === undefined) {
                var i = mainStack.get(control.StackView.index-1)

                if (i)
                    mainStack.pop(i)
            }
        }


        onGroupModify: {
            if (jsonData.error === undefined && jsonData.id === selectedGroupId)
                send("groupGet", {id: selectedGroupId})
        }

        onExamEngineCreate: {
            if (jsonData.error !== undefined) {
                cosClient.sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", qsTr("Létrehozás sikertelen"), jsonData.error)
                return
            }

            if (jsonData.created === true) {
                pushContent(cmpTeacherGroupExamEngine, {
                                title: jsonData.title,
                                code: jsonData.code,
                                mapUuid: jsonData.mapUuid
                            })

                //send("examEngineMapGet", {})
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


    onPageActivated: {
        if (teacherGroups.selectedGroupId > -1) {
            teacherGroups.send("groupGet", {id: teacherGroups.selectedGroupId})
        } else {
            teacherGroups.modelMapList.clear()
        }
    }

}

