import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: control

	closeQuestion: _form.modified ? qsTr("Biztosan eldobod a módosításokat?") : ""

	property User user: null
	property AsyncMessageHandler msgHandler: null

	title: user ? user.fullName : qsTr("Új felhasználó")

	appBar.rightComponent: Qaterial.AppBarButton
	{
		visible: user && msgHandler
		ToolTip.text: qsTr("Felhasználó törlése")
		icon.source: Qaterial.Icons.trashCan
		onClicked: JS.questionDialog(
					   {
						   onAccepted: function()
						   {
							   msgHandler.sendRequestFunc(WebSocketMessage.ClassAdmin, "userRemove", {
															  username: user.username
														  })
						   },
						   text: qsTr("Biztosan törlöd a felhasználót?"),
						   title: user.username,
						   iconSource: Qaterial.Icons.closeCircle
					   })

	}

	QScrollable {
		anchors.fill: parent

		QFormColumn {
			id: _form

			title: qsTr("Felhasználó adatai")

			QFormTextField {
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
			}

			QFormSwitchButton
			{
				id: _active
				text: qsTr("Aktív felhasználó")
			}

			QFormTextField {
				id: _familyName
				title: qsTr("Vezetéknév")
				width: parent.width
				validator: RegExpValidator { regExp: /.+/ }
				errorText: qsTr("Vezetéknév szükséges")
				leadingIconSource: Qaterial.Icons.remoteDesktop
				trailingContent: Qaterial.TextFieldButtonContainer
				{
					Qaterial.TextFieldAlertIcon { visible: _familyName.errorState }
					Qaterial.TextFieldClearButton { visible: _familyName.length; textField: _familyName }
				}
			}

			QFormTextField {
				id: _givenName
				title: qsTr("Keresztnév")
				width: parent.width
				leadingIconSource: Qaterial.Icons.remoteDesktop
				trailingContent: Qaterial.TextFieldClearButton { visible: _givenName.length; textField: _givenName }
			}

			QFormTextField {
				id: _nickName
				title: qsTr("Becenév")
				width: parent.width
				leadingIconSource: Qaterial.Icons.remoteDesktop
				trailingContent: Qaterial.TextFieldClearButton { visible: _nickName.length; textField: _nickName }
			}

			QFormTextField {
				id: _picture
				title: qsTr("Profilkép")
				helperText: qsTr("A profilkép URL címe")
				placeholderText: qsTr("http://...")
				width: parent.width
				validator: RegExpValidator { regExp: /^(http(s):\/\/.)[-a-zA-Z0-9@:%._\+~#=]{2,256}\.[a-z]{2,6}\b([-a-zA-Z0-9@:%_\+.~#?&//=]*)$/ }
				errorText: qsTr("Érvényes URL cím szükséges")
				leadingIconSource: Qaterial.Icons.remoteDesktop
				trailingContent: Qaterial.TextFieldButtonContainer
				{
					Qaterial.TextFieldAlertIcon { visible: _picture.errorState }
					Qaterial.TextFieldClearButton { visible: _picture.length; textField: _picture }
				}
			}


			Qaterial.LabelCaption
			{
				width: parent.width
				wrapMode: Label.Wrap
				text: qsTr("A felhasználó szerepei (jogosultságai):")
				topPadding: 10
			}

			QFormCheckButton
			{
				id: _isTeacher
				text: qsTr("Tanár")
			}

			QFormCheckButton
			{
				id: _isAdmin
				text: qsTr("Admin")
			}

			QFormSwitchButton
			{
				id: _isPanel
				text: qsTr("Virtuális felhasználó (kijelző)")
			}

			QFormTextField {
				id: _password
				width: parent.width
				title: qsTr("Jelszó")
				visible: !user
				echoMode: TextInput.Password
				inputMethodHints: Qt.ImhSensitiveData
				validator: RegExpValidator { regExp: /.+/ }
				errorText: qsTr("Meg kell adni egy jelszót")
				trailingContent: Qaterial.TextFieldPasswordButton { textField: _password }
			}

			QFormTextField {
				id: _password2
				width: parent.width
				visible: !user
				title: qsTr("Jelszó mégegyszer")
				enabled: _password.echoMode == TextInput.Password
				echoMode: TextInput.Password
				inputMethodHints: Qt.ImhSensitiveData
				error: enabled && text !== _password.text
				errorText: qsTr("Nem egyeznek meg a jelszavak")
			}



			QButton
			{
				anchors.horizontalCenter: parent.horizontalCenter
				text: qsTr("Mentés")
				icon.source: Qaterial.Icons.disc
				enabled: msgHandler && _username.acceptableInput && _familyName.acceptableInput && _form.modified &&
						 (user || (_password.acceptableInput &&
						  (_password.echoMode == TextInput.Normal || _password.text == _password2.text)))
				onClicked:
				{
					var d = {}

					if (user)
						d.username = user.username
					else
						d.username = _username.text

					d.familyName = _familyName.text
					d.givenName = _givenName.text
					d.picture = _picture.text
					d.active = _active.checked
					d.isAdmin = _isAdmin.checked
					d.isTeacher = _isTeacher.checked
					d.isPanel = _isPanel.checked

					if (user)
						msgHandler.sendRequestFunc(WebSocketMessage.ClassAdmin, "userModify", d)
					else {
						d.password = _password.text
						msgHandler.sendRequestFunc(WebSocketMessage.ClassAdmin, "userAdd", d)
					}
				}
			}
		}

	}

	Component.onCompleted: if (user) {
							   _username.text = user.username
							   _familyName.text = user.familyName
							   _givenName.text = user.givenName
							   _picture.text = user.picture.toString()
							   _active.checked = user.active
							   _isAdmin.checked = (user.roles & Credential.Admin)
							   _isTeacher.checked = (user.roles & Credential.Teacher)
							   _isPanel.checked = (user.roles & Credential.Panel)
						   } else {
							   _active.checked = true
						   }


	StackView.onActivated: _username.forceActiveFocus()
}
