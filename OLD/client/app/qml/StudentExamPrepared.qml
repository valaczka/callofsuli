import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
    id: control

    title: qsTr("Dolgozatírás előkészítve")
    icon: "qrc:/internal/img/battle.png"

    /*menu: QMenu {
        MenuItem { action: actionList }
    }*/



    /*QIconEmpty {
        visible: userProxyModel.count == 0
        anchors.centerIn: parent
        textWidth: parent.width*0.75
        text: qsTr("Jelenleg egyetlen hadjárat sincs ebben a csoportban")
        tabContainer: control
    }*/

    QLabel {
        id: label
        anchors.centerIn: parent
        text: qsTr("prepared")
        width: parent.width
        wrapMode: Text.Wrap
    }

    Connections {
        target: studentMaps

        function onExamEngineMessage(func, json) {
            label.text += "\n"+func+" - "+JSON.stringify(json)
        }
    }

    onPopulated: {
        studentMaps.send(CosMessage.ClassExamEngine, "prepared", {})
    }
}



