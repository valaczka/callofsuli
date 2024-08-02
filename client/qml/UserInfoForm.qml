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

	property int api: HttpConnection.ApiUser
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


