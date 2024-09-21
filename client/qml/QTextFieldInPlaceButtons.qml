import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Row {
	id: control

	property Qaterial.TextField textField: null
	property string _storedText: ""
	property string setTo: ""
	property bool active: false

	onSetToChanged: set(setTo)
	onTextFieldChanged: if (setTo.length) set(setTo)

	visible: active

	anchors.verticalCenter: parent && (parent instanceof Qaterial.TextFieldButtonContainer) ? parent.verticalCenter : undefined

	signal saveRequest(string text)

	Qaterial.AppBarButton {
		id: saveButton
		icon.source: Qaterial.Icons.check
		foregroundColor: Qaterial.Colors.lightGreen400
		anchors.verticalCenter: parent.verticalCenter
		ToolTip.text: qsTr("Mentés")

		onClicked: {
			if (!textField) {
				console.error("Invalid TextField")
				return
			}

			textField.enabled = false
			control.saveRequest(textField.text)
		}
	}

	Qaterial.AppBarButton {
		icon.source: Qaterial.Icons.close
		foregroundColor: Qaterial.Colors.red400
		anchors.verticalCenter: parent.verticalCenter
		ToolTip.text: qsTr("Mégsem")

		onClicked: revert()
	}

	Connections {
		target: textField
		function onTextEdited() {
			control.active = true
		}

		function onAccepted() {
			if (control.active)
				saveButton.clicked()
		}
	}


	function set(txt) {
		if (!textField)
			return

		_storedText = txt
		textField.text = txt
		control.active = false
	}



	function saved() {
		if (!textField) {
			console.error("Invalid TextField")
			return
		}

		textField.enabled = true
		_storedText = textField.text
		control.active = false
	}

	function revert() {
		if (!textField) {
			console.error("Invalid TextField")
			return
		}

		textField.text = _storedText
		textField.enabled = true
		control.active = false
	}
}
