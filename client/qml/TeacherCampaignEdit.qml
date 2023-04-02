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

	property TeacherGroup group: null
	property Campaign campaign: null

	readonly property string _campaignName: campaign ? campaign.description != "" ? campaign.description :
																					qsTr("Hadjárat #%1").arg(campaign.campaignid) : ""

	title: campaign ? _campaignName : qsTr("Új hadjárat")
	subtitle: group ? group.fullName : ""


	appBar.rightComponent: Qaterial.AppBarButton
	{
		visible: campaign
		ToolTip.text: qsTr("Hadjárat törlése")
		icon.source: Qaterial.Icons.trashCan
		onClicked: JS.questionDialog(
					   {
						   onAccepted: function()
						   {
							   Client.send(WebSocket.ApiTeacher, "campaign/%1/delete".arg(campaign.campaignid))
							   .done(function(r){
								   _form.modified = false
								   Client.stackPop(control)
							   })
							   .fail(JS.failMessage("Törlés sikertelen"))
						   },
						   text: qsTr("Biztosan törlöd a hadjáratot?"),
						   title: _campaignName,
						   iconSource: Qaterial.Icons.closeCircle
					   })

	}



	// ----------------- ITT TARTOK ---------------- (kell egy teacher campaign view -> azon belül stepper (előkészítés - időzítés - start - lezárás)//
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
				icon.source: Qaterial.Icons.disc
				visible: user
				enabled: user && _password._hasAcceptableInput
				onClicked: {
					if (_password.visible) {
						Client.send(WebSocket.ApiAdmin, "user/%1/password".arg(user.username), {
										password: _password.text
									})
						.done(function(r){
							_password.text = ""
							_password2.text = ""
							_password.visible = false
						})
						.fail(JS.failMessage("Jelszóváltoztatás sikertelen"))

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
				validator: RegExpValidator { regExp: /^(http(s)*:\/\/.)[-a-zA-Z0-9@:%._\+~#=]{2,256}\.[a-z]{2,6}\b([-a-zA-Z0-9@:%_\+.~#?&//=]*)$/ }
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





			QButton
			{
				anchors.horizontalCenter: parent.horizontalCenter
				text: qsTr("Mentés")
				icon.source: Qaterial.Icons.disc
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
					d.picture = _picture.text
					d.active = _active.checked
					d.isAdmin = _isAdmin.checked
					d.isTeacher = _isTeacher.checked
					d.isPanel = _isPanel.checked

					if (user) {
						Client.send(WebSocket.ApiAdmin, "user/%1/update".arg(user.username), d)
						.done(function(r){
							_form.modified = false
							Client.stackPop(control)
						})
						.fail(JS.failMessage("Módosítás sikertelen"))
					} else {
						d.password = _password.text
						Client.send(WebSocket.ApiAdmin, "user/create", d)
						.done(function(r){
							_form.modified = false
							Client.stackPop(control)
						})
						.fail(JS.failMessage("Létrehozás sikertelen"))
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
