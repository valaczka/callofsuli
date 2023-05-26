import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


Qaterial.TextArea {
	id: control

	property bool watchModification: true
	readonly property QFormColumn _form : (parent instanceof QFormColumn) ? parent : null
	property string field: ""
	property string fieldData: ""
	readonly property string getData: text

	height: Math.max(implicitHeight, 120 * Qaterial.Style.pixelSizeRatio)

	font: Qaterial.Style.textTheme.body1

	onFieldDataChanged: {
		let t = watchModification
		watchModification = false
		text = fieldData
		watchModification = t
	}

	onTextChanged: if (_form && watchModification) _form.modified = true
}
