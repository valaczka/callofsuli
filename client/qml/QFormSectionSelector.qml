import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS


Column {
    id: control

    property bool watchModification: true
    property QFormColumn form: (parent instanceof QFormColumn) ? parent : null
    property MapEditorStorage storage

    property string field: ""
    property var fieldData: []
    property var getData: function() {
        return fieldData
    }

    width: parent.width

    onFieldDataChanged: update()

    QExpandableHeader {
        width: parent.width
        text: qsTr("Felhasználandó szakaszok")
        icon: Qaterial.Icons.viewGridPlus
        button.visible: false
    }

    Repeater {
        id: _repeater

        model: storage && storage.data.sections ? storage.data.sections : null

        delegate: QFormCheckButton {
            text: modelData.name.length ? modelData.name : qsTr("Szakasz #%1").arg(index+1)
            onToggled: {
                let idx = control.fieldData.indexOf(modelData.key)

                if (idx === -1 && checked)
                    control.fieldData.push(modelData.key)
                else if (idx >= 0 && !checked)
                    control.fieldData.splice(idx, 1)

                if (control.form) control.form.modified = true
            }
        }

        onModelChanged: update()
    }

    function update() {
        for (let i=0; i<_repeater.count; ++i) {
            _repeater.itemAt(i).checked = fieldData.includes(_repeater.model[i].key)
        }
    }
}
