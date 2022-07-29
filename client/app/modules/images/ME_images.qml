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

    property MapEditor mapEditor: null
    property var moduleData: null

    property bool editable: false

    signal modified()

    title: qsTr("Képek")

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

        QGridDoubleImageFields {
            id: fields
            sqlField: "images"

            watchModification: true

            readOnly: !control.editable

            Layout.fillWidth: true
            Layout.columnSpan: layout.columns

            onModification: getData()

            onImageAddRequest: {
                if (!mapEditor)
                    return

                var d = JS.dialogCreateQml("File", {
                                               isSave: false,
                                               folder: cosClient.getSetting("mapFolder", "")
                                           })

                d.item.filters = ["*.jpg", "*.png"]

                d.accepted.connect(function(data){
                    var i = mapEditor.imageAdd(data)

                    if (i !== -1) {
                        imageAddRequestSuccess(i)
                    }

                    cosClient.setSetting("mapFolder", d.item.modelFolder)
                })

                d.open()
            }

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




