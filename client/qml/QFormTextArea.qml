import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


Qaterial.TextArea {
	id: control

	readonly property QFormColumn _form : (parent instanceof QFormColumn) ? parent : null
	property string field: ""
	property string fieldData: ""
	readonly property string getData: text

	onFieldDataChanged: text = fieldData

	onTextChanged: if (_form) _form.modified = true
}
