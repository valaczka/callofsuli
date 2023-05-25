import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


Qaterial.CheckButton {
	id: control

	readonly property QFormColumn _form : (parent instanceof QFormColumn) ? parent : null
	property string field: ""
	property bool fieldData: false
	readonly property bool getData: checked

	onFieldDataChanged: checked = fieldData

	onToggled: if (_form) _form.modified = true
}
