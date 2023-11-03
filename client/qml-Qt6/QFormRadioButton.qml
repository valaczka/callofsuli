import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


Qaterial.RadioButton {
	id: control

	readonly property QFormColumn _form : (parent instanceof QFormColumn) ? parent : null
	property string field: ""
	property bool fieldData: false
	property var getData: function() { return checked }

	onFieldDataChanged: checked = fieldData

	onToggled: if (_form) _form.modified = true
}
