import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


Qaterial.TextArea {
	id: root

	property QFormBindingField bindingField: null
	property var saveData: function(d) { return text }

	readOnly: bindingField && bindingField.readOnly

	height: Math.max(implicitHeight, 120 * Qaterial.Style.pixelSizeRatio)
	font: Qaterial.Style.textTheme.body1

	signal gotoNextField()

	onEditingFinished: if (bindingField)
						   bindingField.performModification()

	function loadData(d) {
		text = d
	}

	function performGoto() {
		root.forceActiveFocus()
		return true
	}

}
