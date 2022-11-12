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

    property alias cmpTeacherCampaignDetails: cmpTeacherCampaignDetails
    property alias cmpTeacherCampaignResult: cmpTeacherCampaignResult


    title: teacherGroups.selectedGroupFullName

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
        id: cmpTeacherCampaignDetails
        TeacherGroupCampaignDetails {  }
    }

    Component {
        id: cmpTeacherCampaignResult
        TeacherGroupCampaignResult {  }
    }

    Component {
        id: cmpTeacherGroupMain
        TeacherGroupMain { }
    }


    onPageActivated: {
        if (teacherGroups.selectedGroupId > -1) {
            teacherGroups.send("groupGet", {id: teacherGroups.selectedGroupId})
        } else {
            teacherGroups.modelMapList.clear()
        }
    }

    Component.onCompleted: {
        replaceContent(cmpTeacherGroupMain)
    }

}

