import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QFormColumn {
	id: _form

	spacing: 3

	property bool editable: false
	property bool nameEditable: true
	property bool pictureEditable: false

	property int api: WebSocket.ApiUser
	property string path: "update"

	QFormTextField {
		id: _username
		title: qsTr("Felhasználónév")
		width: parent.width
		readOnly: true
		leadingIconSource: Qaterial.Icons.remoteDesktop
		field: "username"
	}


	QFormTextField {
		id: _familyName
		title: qsTr("Vezetéknév")
		field: "familyName"
		width: parent.width
		readOnly: !_form.editable || !_form.nameEditable
		validator: RegExpValidator { regExp: /.+/ }
		errorText: qsTr("Vezetéknév szükséges")
		leadingIconSource: Qaterial.Icons.remoteDesktop
		trailingContent: Qaterial.TextFieldButtonContainer
		{
			Qaterial.TextFieldAlertIcon { visible: _familyName.errorState }
			Qaterial.TextFieldClearButton { visible: !_familyName.readOnly && _familyName.length; textField: _familyName }
		}
	}

	QFormTextField {
		id: _givenName
		title: qsTr("Keresztnév")
		field: "givenName"
		width: parent.width
		readOnly: !_form.editable || !_form.nameEditable
		leadingIconSource: Qaterial.Icons.remoteDesktop
		trailingContent: Qaterial.TextFieldClearButton { visible: !_givenName.readOnly && _givenName.length; textField: _givenName }
	}

	QFormTextField {
		id: _nickName
		title: qsTr("Becenév")
		field: "nickName"
		width: parent.width
		readOnly: !_form.editable
		leadingIconSource: Qaterial.Icons.remoteDesktop
		trailingContent: Qaterial.TextFieldClearButton { visible: _nickName.length; textField: _nickName }
	}

	QFormTextField {
		id: _picture
		title: qsTr("Kép")
		field: "picture"
		width: parent.width
		readOnly: !_form.editable || !_form.pictureEditable
		leadingIconSource: Qaterial.Icons.imageOutline
		trailingContent: Qaterial.TextFieldClearButton { visible: _picture.length; textField: _picture }
	}




	/*
	Row {
		anchors.left: parent.left

		spacing: 15

		Qaterial.LabelBody2 {
			text: qsTr("Karakter:")
			anchors.verticalCenter: parent.verticalCenter
		}

		MouseArea {
			id: _area
			width: _terrainRow.implicitWidth
			height: _terrainRow.implicitHeight

			hoverEnabled: true

			acceptedButtons: Qt.LeftButton

			onClicked: _terrainAddButton.clicked()

			Row {
				id: _terrainRow

				spacing: 15

				Qaterial.Icon
				{
					icon: missionLevel ? (missionLevel.terrainData.name !== "" ? missionLevel.terrainData.thumbnail : Qaterial.Icons.alert): ""
					color: missionLevel && missionLevel.terrainData.name !== "" ? "transparent" : Qaterial.Colors.red500
					width: missionLevel && missionLevel.terrainData.name !== "" ? Qaterial.Style.pixelSize*4.5 : Qaterial.Style.mediumIcon
					height: missionLevel && missionLevel.terrainData.name !== "" ? Qaterial.Style.pixelSize*3 : Qaterial.Style.mediumIcon
				}

				Qaterial.LabelBody1 {
					anchors.verticalCenter: parent.verticalCenter
					text: "????IIIII"
				}

				Qaterial.RoundButton {
					id: _terrainAddButton
					anchors.verticalCenter: parent.verticalCenter
					icon.source: Qaterial.Icons.pencil
					enabled: missionLevel
					onClicked: {
						let idx = -1

						for (let i=0; i<_sortedTerrainModel.count; ++i) {
							if (_sortedTerrainModel.get(i).fieldName === missionLevel.terrain)
								idx = i
						}

						Qaterial.DialogManager.openListView(
									{
										onAccepted: function(index)
										{
											if (index < 0)
												return

											editor.missionLevelModify(missionLevel, function() {
												missionLevel.terrain = _sortedTerrainModel.get(index).fieldName
											})

										},
										title: qsTr("Harcmező kiválasztása"),
										model: _sortedTerrainModel,
										currentIndex: idx,
										delegate: _terrainDelegate
									})
					}
				}
			}

			Qaterial.ListDelegateBackground
			{
				anchors.fill: parent
				type: Qaterial.Style.DelegateType.Icon
				lines: 1
				pressed: _area.pressed
				rippleActive: _area.containsMouse
				rippleAnchor: _area
			}
		}
	}
*/




	function loadData(user) {
		_form.setItems([_username, _familyName, _givenName, _nickName, _picture], user)
		_form.modified = false
	}

	QButton
	{
		anchors.horizontalCenter: parent.horizontalCenter
		text: qsTr("Mentés")
		icon.source: Qaterial.Icons.contentSave
		visible: _form.editable
		enabled: _form.modified && (!nameEditable || _familyName.acceptableInput)

		onClicked:
		{
			var d = {}

			if (nameEditable) {
				d.familyName = _familyName.text
				d.givenName = _givenName.text
			}

			d.nickname = _nickName.text

			if (pictureEditable)
				d.picture = _picture.text

			_form.enabled = false

			Client.send(api, path, d)
			.done(function(r){
				_form.enabled = true
				Client.snack(qsTr("Módosítás sikeres"))
				Client.reloadUser(function() { loadData(Client.userToMap(Client.server.user)) })
			})
			.fail(function(err) {
				Client.messageWarning(err, qsTr("Módosítás sikertelen"))
				_form.enabled = true
			})
			.error(function(err) {
				_form.enabled = true
			})

		}
	}
}


