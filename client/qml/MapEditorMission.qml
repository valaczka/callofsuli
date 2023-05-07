import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: root

	stackPopFunction: function() {
		/*if (_campaignList.view.selectEnabled) {
			_campaignList.view.unselectAll()
			return false
		}

		if (swipeView.currentIndex > 0) {
			swipeView.setCurrentIndex(0)
			return false
		}*/

		return true
	}

	title: mission ? mission.name : ""
	subtitle: editor ? editor.displayName : ""

	property MapEditor editor: null
	property MapEditorMission mission: null


	appBar.backButtonVisible: true
	appBar.rightComponent: MapEditorToolbarComponent {
		editor: root.editor
	}



	QScrollable {
		anchors.fill: parent

		QFormColumn {
			id: _form

			Row {
				width: parent.width
				spacing: 10

				Qaterial.RawMaterialButton {
					id: _buttonMedal
					icon.source: mission.fullMedalImage
					icon.color: "transparent"
					icon.width: 2.2 * Qaterial.Style.pixelSize
					icon.height: 2.2 * Qaterial.Style.pixelSize
					width: 3.5 * Qaterial.Style.pixelSize
					height: 3.5 * Qaterial.Style.pixelSize
					flat: true
					outlined: false

					anchors.verticalCenter: parent.verticalCenter
				}

				Qaterial.TextField {
					id: _textFieldName
					visible: mission
					enabled: mission

					width: parent.width-parent.spacing-_buttonMedal.width

					anchors.verticalCenter: parent.verticalCenter

					font: Qaterial.Style.textTheme.headline4
					placeholderText: qsTr("A küldetés neve")
					backgroundBorderHeight: 1
					backgroundColor: "transparent"
					text: mission ? mission.name : ""

					onEditingFinished: editor.missionModify(mission, function() {
						mission.name = text
					})

				}
			}


			/*QFormTextField {
				id: _username
				title: qsTr("Felhasználónév")
				width: parent.width
				readOnly: user
				helperText: qsTr("Egyedi felhasználónév, email cím")
				validator: RegExpValidator { regExp: /.+/ }
				errorText: qsTr("Felhasználónév szükséges")
				leadingIconSource: Qaterial.Icons.remoteDesktop
				trailingContent: Qaterial.TextFieldButtonContainer
				{
					Qaterial.TextFieldAlertIcon { visible: _username.errorState }
					Qaterial.TextFieldClearButton { visible: _username.length && !_username.readOnly; textField: _username }
				}
			}*/

			QFormTextArea {
				id: _textAreaDescription
				width: parent.width
				title: qsTr("Leírás")
				placeholderText: qsTr("Rövid tájékoztató a küldetésről")
				height: Math.max(implicitHeight, 150)
				text: mission ? mission.description : ""
				font: Qaterial.Style.textTheme.body1

				onEditingFinished: editor.missionModify(mission, function() {
					mission.description = text
				})

				Connections {
					target: mission

					function onDescriptionChanged() {
						_textAreaDescription.text = mission.description
					}
				}

			}



			Qaterial.LabelCaption
			{
				width: parent.width
				wrapMode: Label.Wrap
				text: qsTr("Lehetséges játékmódok:")
				topPadding: 10
			}

			QFormCheckButton
			{
				id: _isAction
				text: qsTr("Akciójáték")
				checked: mission && (mission.modes & GameMap.Action)
				onToggled: _form.updateCheckButtons()
			}

			QFormCheckButton
			{
				id: _isLite
				text: qsTr("Feladatmegoldás")
				checked: mission && (mission.modes & GameMap.Lite)
				onToggled: _form.updateCheckButtons()
			}

			QFormCheckButton
			{
				id: _isTest
				text: qsTr("Teszt")
				checked: mission && (mission.modes & GameMap.Test)
				onToggled: _form.updateCheckButtons()
			}

			function updateCheckButtons() {
				var c = 0

				if (_isAction.checked)
					c |= GameMap.Action

				if (_isLite.checked)
					c |= GameMap.Lite

				if (_isTest.checked)
					c |= GameMap.Test

				editor.missionModify(mission, function() {
					mission.modes = c
				})
			}
		}

	}


	onMissionChanged: if (!mission) Client.stackPop(root)

	StackView.onActivated: _textFieldName.forceActiveFocus()
}
