import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


Qaterial.TextField {
	id: root

	property QFormBindingField bindingField: null
	property var saveData: function(d) { return text }

	readOnly: bindingField && bindingField.readOnly

	signal gotoNextField()

	onTextEdited: if (bindingField)
					  bindingField.performModification()

	onAccepted: {
		if (!readOnly)
			gotoNextField()
	}

	function loadData(d) {
		text = d
	}


	function performGoto() {
		root.forceActiveFocus()
		return true
	}

}
