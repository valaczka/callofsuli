import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QCollapsible {
    id: control

    collapsed: false

    property var moduleData: null

    property bool editable: false

    signal modified()

    title: qsTr("Összerendelések")

    rightComponent: QToolButton {
        visible: !control.editable
        icon.source: "qrc:/internal/icon/pencil.svg"
        text: qsTr("Szerkesztés")
        display: AbstractButton.IconOnly
        onClicked: control.editable = true
    }

    QGridLayout {
        id: layout

        watchModification: true
        onModifiedChanged: if (layout.modified)
                               control.modified()

        QGridDoubleTextFields {
            id: fields
            sqlField: "bindings"

            watchModification: true

            readOnly: !control.editable

            Layout.fillWidth: true
            Layout.columnSpan: layout.columns

            onModification: getData()

        }
    }

    Component.onCompleted: {
        if (!moduleData)
            return

        JS.setSqlFields([fields], moduleData)
    }


    function getData() {
        moduleData = JS.getSqlFields([fields])

        if (editable)
            return moduleData
        else
            return {}
    }

}




