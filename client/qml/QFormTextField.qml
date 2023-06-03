import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


Qaterial.TextField {
	id: control

	property bool watchModification: true
	readonly property QFormColumn _form : (parent instanceof QFormColumn) ? parent : null
	property string field: ""
	property string fieldData: ""
	property var getData: function() { return text }

	onFieldDataChanged: text = fieldData

	onTextEdited: if (_form && watchModification) _form.modified = true
}
