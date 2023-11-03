import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
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
		//leadingIconSource: Qaterial.Icons.remoteDesktop
		field: "username"
		helperText: qsTr("A felhasználónév nem módosítható")
	}


	QFormTextField {
		id: _familyName
		title: qsTr("Vezetéknév")
		field: "familyName"
		width: parent.width
		readOnly: !_form.editable || !_form.nameEditable
		helperText: !_form.nameEditable ? qsTr("A vezetéknevek módosítása nincs engedélyezve a szerveren") : ""
		validator: RegularExpressionValidator { regularExpression: /.+/ }
		errorText: qsTr("Vezetéknév szükséges")
		//leadingIconSource: Qaterial.Icons.remoteDesktop
		trailingContent: Qaterial.TextFieldButtonContainer
		{
			Qaterial.TextFieldAlertIcon {  }
			Qaterial.TextFieldClearButton { visible: !_familyName.readOnly && _familyName.length }
		}
	}

	QFormTextField {
		id: _givenName
		title: qsTr("Keresztnév")
		field: "givenName"
		width: parent.width
		readOnly: !_form.editable || !_form.nameEditable
		helperText: !_form.nameEditable ? qsTr("A keresztnevek módosítása nincs engedélyezve a szerveren") : ""
		//leadingIconSource: Qaterial.Icons.remoteDesktop
		trailingContent: Qaterial.TextFieldButtonContainer
		{
			Qaterial.TextFieldClearButton { visible: !_givenName.readOnly && _givenName.length }
		}
	}

	QFormTextField {
		id: _nickName
		title: qsTr("Becenév")
		field: "nickName"
		width: parent.width
		readOnly: !_form.editable
		//leadingIconSource: Qaterial.Icons.remoteDesktop
		trailingContent: Qaterial.TextFieldButtonContainer
		{
			Qaterial.TextFieldClearButton { visible: !_nickName.readOnly && _nickName.length }
		}
	}

	QFormTextField {
		id: _picture
		title: qsTr("Kép")
		field: "picture"
		width: parent.width
		readOnly: !_form.editable || !_form.pictureEditable
		helperText: !_form.nameEditable ? qsTr("A képek módosítása nincs engedélyezve a szerveren") : ""
		//leadingIconSource: Qaterial.Icons.imageOutline
		trailingContent: Qaterial.TextFieldButtonContainer
		{
			Qaterial.TextFieldClearButton { visible: !_picture.readOnly && _picture.length }
		}
	}





	Row {
		anchors.left: parent.left
		topPadding: 5

		spacing: 15

		Qaterial.LabelBody2 {
			text: qsTr("Karakter:")
			anchors.verticalCenter: parent.verticalCenter
		}

		MouseArea {
			id: _area
			width: _characterRow.implicitWidth
			height: _characterRow.implicitHeight

			anchors.verticalCenter: parent.verticalCenter

			hoverEnabled: true

			acceptedButtons: Qt.LeftButton

			onClicked: _changeBtn.clicked()

			Row {
				id: _characterRow

				property string character: "default"

				spacing: 15

				Qaterial.Icon
				{
					icon: "qrc:/character/%1/thumbnail.png".arg(_characterRow.character)
					color: "transparent"
					width: Qaterial.Style.largeIcon
					height: Qaterial.Style.largeIcon
					anchors.verticalCenter: parent.verticalCenter
				}

				Qaterial.LabelBody1 {
					anchors.verticalCenter: parent.verticalCenter
					text: Client.availableCharacters()[_characterRow.character].name
					color: Qaterial.Style.foregroundColor
				}

				Qaterial.RoundButton {
					id: _changeBtn
					anchors.verticalCenter: parent.verticalCenter
					icon.source: Qaterial.Icons.pencil
					onClicked: {
						let idx = -1

						let list = Object.keys(Client.availableCharacters())

						let model = []


						for (let i=0; i<list.length; ++i) {
							model.push({
										   id: list[i],
										   name: Client.availableCharacters()[list[i]].name
									   })

							if (list[i] === _characterRow.character)
								idx = i
						}

						Qaterial.DialogManager.openListView(
									{
										onAccepted: function(index)
										{
											if (index < 0)
												return

											_characterRow.character = model[index].id
											_form.modified = true
										},
										title: qsTr("Karakter kiválasztása"),
										model: model,
										currentIndex: idx,
										delegate: _characterDelegate
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


	Component {
		id: _characterDelegate

		Qaterial.LoaderItemDelegate {
			highlighted: ListView.isCurrentItem
			text: modelData ? modelData.name : ""

			leftSourceComponent: Image
			{
				fillMode: Image.PreserveAspectFit
				source: modelData ? "qrc:/character/%1/thumbnail.png".arg(modelData.id) : ""
				sourceSize: Qt.size(width, height)
			}

			width: ListView.view.width
			onClicked: ListView.view.select(index)
		}
	}





	function loadData(user) {
		_form.setItems([_username, _familyName, _givenName, _nickName, _picture], user)

		if (Client.availableCharacters()[user.character] !== undefined)
			_characterRow.character = user.character
		else
			_characterRow.character = "default"

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
			d.character = _characterRow.character

			if (pictureEditable)
				d.picture = _picture.text

			_form.enabled = false

			Client.send(api, path, d)
			.done(_form, function(r){
				_form.enabled = true
				Client.snack(qsTr("Módosítás sikeres"))
				Client.reloadUser(function() { loadData(Client.userToMap(Client.server.user)) })
			})
			.fail(_form, function(err) {
				Client.messageWarning(err, qsTr("Módosítás sikertelen"))
				_form.enabled = true
			})
			.error(_form, function(err) {
				_form.enabled = true
			})

		}
	}
}


