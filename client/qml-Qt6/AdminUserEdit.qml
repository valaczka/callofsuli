import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPage {
	id: control

	closeQuestion: _form.modified ? qsTr("Biztosan eldobod a módosításokat?") : ""

	property User user: null
	property int classid: -1

	title: user ? user.fullName : qsTr("Új felhasználó")

	appBar.rightComponent: Qaterial.AppBarButton
	{
		visible: user
		ToolTip.text: qsTr("Felhasználó törlése")
		icon.source: Qaterial.Icons.delete_
		onClicked: JS.questionDialog(
					   {
						   onAccepted: function()
						   {
							   Client.send(WebSocket.ApiAdmin, "user/%1/delete".arg(user.username))
							   .done(control, function(r){
								   _form.modified = false
								   Client.stackPop(control)
							   })
							   .fail(control, JS.failMessage("Törlés sikertelen"))
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
				validator: RegularExpressionValidator { regularExpression: /.+/ }
				errorText: qsTr("Felhasználónév szükséges")
				leadingIconSource: Qaterial.Icons.accountOutline
				trailingContent: Qaterial.TextFieldButtonContainer
				{
					Qaterial.TextFieldAlertIcon {  }
					Qaterial.TextFieldClearButton { visible: _username.length && !_username.readOnly }
				}
			}

			QFormTextField {
				id: _password
				width: parent.width
				title: qsTr("Jelszó")
				visible: !user
				echoMode: TextInput.Password
				inputMethodHints: Qt.ImhSensitiveData
				validator: RegularExpressionValidator { regularExpression: /.+/ }
				errorText: qsTr("Meg kell adni egy jelszót")
				trailingContent: Qaterial.TextFieldButtonContainer
				{
					Qaterial.TextFieldPasswordButton {  }
				}
				watchModification: false

				readonly property bool _hasAcceptableInput: !visible ||
															(acceptableInput && (echoMode == TextInput.Normal || text == _password2.text))
			}

			QFormTextField {
				id: _password2
				width: parent.width
				visible: _password.visible
				title: qsTr("Jelszó mégegyszer")
				enabled: _password.echoMode == TextInput.Password
				echoMode: TextInput.Password
				inputMethodHints: Qt.ImhSensitiveData
				error: enabled && text !== _password.text
				errorText: qsTr("Nem egyeznek meg a jelszavak")
				watchModification: false
			}

			QButton
			{
				anchors.horizontalCenter: parent.horizontalCenter
				text: _password.visible ? qsTr("Jelszó mentése") : qsTr("Új jelszó beállítása")
				icon.source: _password.visible ? Qaterial.Icons.contentSave : Qaterial.Icons.shieldCheck
				visible: user
				enabled: user && _password._hasAcceptableInput
				onClicked: {
					if (_password.visible) {
						Client.send(WebSocket.ApiAdmin, "user/%1/password".arg(user.username), {
										password: _password.text
									})
						.done(control, function(r){
							_password.text = ""
							_password2.text = ""
							_password.visible = false
						})
						.fail(control, JS.failMessage("Jelszóváltoztatás sikertelen"))

					} else {
						_password.visible = true
					}
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
				validator: RegularExpressionValidator { regularExpression: /.+/ }
				errorText: qsTr("Vezetéknév szükséges")
				trailingContent: Qaterial.TextFieldButtonContainer
				{
					Qaterial.TextFieldAlertIcon {  }
					Qaterial.TextFieldClearButton {  }
				}
			}

			QFormTextField {
				id: _givenName
				title: qsTr("Keresztnév")
				width: parent.width
				trailingContent: Qaterial.TextFieldButtonContainer
				{

					Qaterial.TextFieldClearButton {  }
				}
			}

			QFormTextField {
				id: _nickName
				title: qsTr("Becenév")
				width: parent.width
				trailingContent: Qaterial.TextFieldButtonContainer
				{
					Qaterial.TextFieldClearButton {  }
				}
			}

			QFormTextField {
				id: _picture
				title: qsTr("Profilkép")
				helperText: qsTr("A profilkép URL címe")
				placeholderText: qsTr("http://...")
				width: parent.width
				validator: RegularExpressionValidator { regularExpression: /^(http(s)*:\/\/.)[-a-zA-Z0-9@:%._\+~#=]{2,256}\.[a-z]{2,6}\b([-a-zA-Z0-9@:%_\+.~#?&//=]*)$/ }
				errorText: qsTr("Érvényes URL cím szükséges")
				leadingIconSource: Qaterial.Icons.image
				trailingContent: Qaterial.TextFieldButtonContainer
				{
					Qaterial.TextFieldAlertIcon {  }
					Qaterial.TextFieldClearButton {  }
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





			QButton
			{
				anchors.horizontalCenter: parent.horizontalCenter
				text: qsTr("Mentés")
				icon.source: Qaterial.Icons.contentSave
				enabled: _username.acceptableInput && _familyName.acceptableInput
						 && _form.modified && (user || _password._hasAcceptableInput)
				onClicked:
				{
					var d = {}

					if (user)
						d.username = user.username
					else {
						d.username = _username.text
						d.classid = control.classid
					}

					d.familyName = _familyName.text
					d.givenName = _givenName.text
					d.nickname = _nickName.text
					d.picture = _picture.text
					d.active = _active.checked
					d.isAdmin = _isAdmin.checked
					d.isTeacher = _isTeacher.checked
					d.isPanel = _isPanel.checked

					if (user) {
						Client.send(WebSocket.ApiAdmin, "user/%1/update".arg(user.username), d)
						.done(control, function(r){
							_form.modified = false
							Client.stackPop(control)
						})
						.fail(control, JS.failMessage("Módosítás sikertelen"))
					} else {
						d.password = _password.text
						Client.send(WebSocket.ApiAdmin, "user/create", d)
						.done(control, function(r){
							_form.modified = false
							Client.stackPop(control)
						})
						.fail(control, JS.failMessage("Létrehozás sikertelen"))
					}
				}
			}
		}

	}


	Component.onCompleted: if (user) {
							   _username.text = user.username
							   _familyName.text = user.familyName
							   _givenName.text = user.givenName
							   _nickName.text = user.nickName
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
