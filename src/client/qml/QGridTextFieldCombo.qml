import QtQuick 2.12
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import "Style"

Row {
	id: control
	property string fieldName: ""
	property string sqlField: ""
	property string sqlData: field.text+combo.textAt(combo.currentIndex)
	property bool modified: false

	readonly property string text: field.text+combo.textAt(combo.currentIndex)

	property bool watchModification: parent.watchModification

	property alias textField: field
	property alias comboBox: combo

	Layout.fillWidth: true
	Layout.bottomMargin: parent.columns === 1 ? 10 : 0

	spacing: 0

	QTextField {
		id: field
		//	signal applied()

		width: parent.width-combo.width

		placeholderText: control.parent.columns > 1 ? "" : control.fieldName

		onTextEdited:  if (control.watchModification) {
						   control.modified = true
						   control.parent.modified = true
					   }


		onAccepted: control.parent.accept()

	}

	QComboBox {
		id: combo

		//width: Math.min(implicitWidth, control.width/2)

		onActivated: {
			if (control.watchModification) {
				control.modified = true
				control.parent.modified = true
			}
		}

	}

	function setValue(t) {
		var i = combo.indexOfValue(t)
		if (i === -1)
			combo.currentIndex = 0
		else
			combo.currentIndex = i
		modified = false
	}

	function setText(t) {
		field.text = t
		modified = false
		//		_watch = true
	}

}
