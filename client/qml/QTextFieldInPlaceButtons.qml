import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Row {
	id: control

	property Qaterial.TextField textField: null
	property Qaterial.TextArea textArea: null
	property string _storedText: ""
	property string setTo: ""
	property bool active: false

	onSetToChanged: set(setTo)
	onTextFieldChanged: if (setTo.length) set(setTo)
	onTextAreaChanged: if (setTo.length) set(setTo)

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
			if (textArea) {
				textArea.enabled = false
				control.saveRequest(textArea.text)
				return
			}

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

	Connections {
		target: textArea ? textArea.textArea : null

		function onTextEdited() {
			control.active = true
		}

		function onEditingFinished() {
			control.active = true
		}
	}


	function set(txt) {
		if (textArea) {
			_storedText = txt
			textArea.text = txt
			control.active = false
			return
		}

		if (!textField)
			return

		_storedText = txt
		textField.text = txt
		control.active = false
	}



	function saved() {
		if (textArea) {
			textArea.enabled = true
			_storedText = textArea.text
			control.active = false
			return
		}

		if (!textField) {
			console.error("Invalid TextField")
			return
		}

		textField.enabled = true
		_storedText = textField.text
		control.active = false
	}

	function revert() {
		if (textArea) {
			textArea.text = _storedText
			textArea.enabled = true
			control.active = false
			return
		}

		if (!textField) {
			console.error("Invalid TextField")
			return
		}

		textField.text = _storedText
		textField.enabled = true
		control.active = false
	}
}
