import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QTabPage {
    id: control

    contentTitle: qsTr("Dolgozatírás")

    activity: StudentMaps {
        id: studentMaps

        property string examEngine: ""
        property string mapUuid: ""

        onMapDownloadRequest: {
            var dd = JS.dialogCreateQml("Progress", { title: qsTr("Letöltés"), downloader: studentMaps.downloader })
            dd.closePolicy = Popup.NoAutoClose
            dd.open()
        }


        onGameMapLoaded: {
            replaceContent(cmpStudentExamPrepared)
        }

        onGameMapUnloaded: {
            if (control.StackView.view)
                mainStack.pop(control)
        }

        onExamEngineConnect: {
            if (jsonData.error !== undefined) {
                cosClient.sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", qsTr("Sikertelen csatlakozás"), jsonData.error)
                return
            }

            if (jsonData.connected === true) {
                examEngine = jsonData.code
                mapUuid = jsonData.mapUuid
                control.contentTitle = jsonData.title

                send("examEngineMapGet", {})
            }
        }


        onExamEngineMessage: {
            if (func === "engineStopped" && jsonData.code === examEngine) {
                examEngine = ""
                replaceContent(cmpStudentExamFinished)
            }
        }

        Component.onCompleted: init(false)
    }



    Component {
        id: cmpStudentExamConnect
        StudentExamConnect {  }
    }

    Component {
        id: cmpStudentExamPrepared
        StudentExamPrepared {  }
    }

    Component {
        id: cmpStudentExamFinished
        StudentExamFinished { }
    }


    Component.onCompleted: {
        pushContent(cmpStudentExamConnect)
    }

    pageBackCallbackFunction: function () {
        if (studentMaps.examEngine != "") {
            cosClient.sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", qsTr("Dolgozatírás folyamatban"),
                                              qsTr("Amíg a dolgozatírás tart, nem lehet kilépni!"))
            return true
        }

        return false
    }

    closeCallbackFunction: function () {
        if (windowCloseFunction())
            return true

        if (studentMaps.examEngine != "") {
            cosClient.sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", qsTr("Dolgozatírás folyamatban"),
                                              qsTr("Amíg a dolgozatírás tart, nem lehet kilépni!"))
            return true
        }

        return false
    }
}

