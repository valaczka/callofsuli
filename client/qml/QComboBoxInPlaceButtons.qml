import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Row {
	id: control

	property ComboBox combo: null
	property int _storedIndex: -1
	property int setTo: -1
	property bool active: false

	onSetToChanged: set(setTo)
	onComboChanged: if (setTo > -1) set(setTo)

	visible: active

	signal saveRequest(int index)

	Qaterial.AppBarButton {
		id: saveButton
		icon.source: Qaterial.Icons.check
		foregroundColor: Qaterial.Colors.lightGreen400
		anchors.verticalCenter: parent.verticalCenter
		ToolTip.text: qsTr("Mentés")

		onClicked: {
			if (!combo) {
				console.error("Invalid ComboBox")
				return
			}

			combo.enabled = false
			control.saveRequest(combo.currentIndex)
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
		target: combo
		function onActivated() {
			control.active = true
		}

		function onAccepted() {
			if (control.active)
				saveButton.clicked()
		}
	}


	function set(idx) {
		if (!combo)
			return

		_storedIndex = idx
		combo.currentIndex = idx
		control.active = false
	}



	function saved() {
		if (!combo) {
			console.error("Invalid ComboBox")
			return
		}

		combo.enabled = true
		_storedIndex = combo.currentIndex
		control.active = false
	}

	function revert() {
		if (!combo) {
			console.error("Invalid ComboBox")
			return
		}

		combo.currentIndex = _storedIndex
		combo.enabled = true
		control.active = false
	}
}
