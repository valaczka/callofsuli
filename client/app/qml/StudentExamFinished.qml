import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
    id: control

    title: qsTr("Dolgozatírás")
    icon: "qrc:/internal/img/battle.png"


    QIconEmpty {
        anchors.centerIn: parent
        textWidth: parent.width*0.75
        text: qsTr("A dolgozatírás befejeződött")
        tabContainer: control
        color: CosStyle.colorOKLighter
    }

}



