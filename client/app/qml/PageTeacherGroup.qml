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
        /*ListElement {
            title: qsTr("Eredmények")
            icon: "image://font/AcademicI/\uf15d"
            iconColor: "tomato"
            func: function() { replaceContent(cmpTeacherGroupScore) }
        }*/
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


        /*onCampaignListGet: {
            if (!actionCampaignFilter.waitForList)
                return

            actionCampaignFilter.waitForList = false

            modelCampaignFilterList.clear()

            for (var i=0; i<jsonData.list.length; i++) {
                var o = jsonData.list[i]
                o.subtitle = JS.readableInterval(o.starttime, o.endtime)
                modelCampaignFilterList.append(o)
            }

            modelCampaignFilterList.append({
                                               id: -1,
                                               description: qsTr("-- Nincs szűrés --"),
                                               subtitle: "",
                                               starttime: "",
                                               endtime: "",
                                               finsihed: false
                                           })

            var d = JS.dialogCreateQml("List", {
                                           icon: "qrc:/internal/icon/filter.svg",
                                           title: qsTr("Szűrés hadjáratra"),
                                           selectorSet: false,
                                           modelTitleRole: "title",
                                           modelSubtitleRole: "subtitle",
                                           modelIconRole: "icon",
                                           modelIconColorRole: "iconcolor",
                                           modelTitleColorRole: "iconcolor",
                                           modelSubtitleColorRole: "iconcolor",
                                           delegateHeight: CosStyle.twoLineHeight,
                                           imageWidth: 0,
                                           model: listCampaignFilterModel
                                       })


            d.accepted.connect(function(data) {
                if (!data)
                    return

                actionCampaignFilter.campaign = data.id
            })
            d.open()
        }*/
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
        id: cmpTeacherGroupScore
        TeacherGroupScore { }
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

