import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QFormColumn {
	id: control

	visible: Client.server && Client.server.user.loginState == User.LoggedOut &&
			 Client.server.config.registrationEnabled !== undefined && Client.server.config.registrationEnabled

	title: qsTr("Adatok")

	property alias code: _code.text
	property alias plainChecked: _radioPlain.checked
	property alias googleChecked: _radioGoogle.checked
	property alias button: button

	QFormTextField {
		id: _code
		title: qsTr("Hitelesítő kód")
		width: parent.width
		helperText: qsTr("A regisztrációhoz kapott hitelesítő kód")
		validator: RegExpValidator { regExp: /.+/ }
		errorText: qsTr("Meg kell adni a hitelesítő kódot")
		leadingIconSource: Qaterial.Icons.remoteDesktop
		trailingContent: Qaterial.TextFieldButtonContainer
		{
			Qaterial.TextFieldAlertIcon { visible: _code.errorState }
			Qaterial.TextFieldClearButton { visible: _code.length; textField: _code }
		}
	}

	Qaterial.LabelCaption
	{
		width: parent.width
		wrapMode: Label.Wrap
		text: qsTr("Válaszd ki, hogyan szeretnél regisztrálni:")
		topPadding: 10
	}

	QFormRadioButton {
		id: _radioPlain
		text: qsTr("Regisztráció felhasználónévvel, jelszóval")
	}

	QFormRadioButton {
		id: _radioGoogle
		text: qsTr("Regisztráció Google fiókkal")
		checked: true
	}

	QFormTextField {
		id: _username
		title: qsTr("Felhasználónév")
		width: parent.width
		visible: _radioPlain.checked
		validator: RegExpValidator { regExp: /.+/ }
		errorText: qsTr("Meg kell adni egy felhasználónevet")
		leadingIconSource: Qaterial.Icons.remoteDesktop
		trailingContent: Qaterial.TextFieldButtonContainer
		{
			Qaterial.TextFieldAlertIcon { visible: _username.errorState }
			Qaterial.TextFieldClearButton { visible: _username.length; textField: _username }
		}
	}


	QFormTextField {
		id: _familyName
		title: qsTr("Vezetéknév")
		width: parent.width
		visible: _radioPlain.checked
		validator: RegExpValidator { regExp: /.+/ }
		errorText: qsTr("Meg kell adni a vezetéknevet")
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
		visible: _radioPlain.checked
		leadingIconSource: Qaterial.Icons.remoteDesktop
		trailingContent: Qaterial.TextFieldButtonContainer
		{
			Qaterial.TextFieldAlertIcon { visible: _givenName.errorState }
			Qaterial.TextFieldClearButton { visible: _givenName.length; textField: _givenName }
		}
	}

	QFormTextField {
		id: _password
		width: parent.width
		title: qsTr("Jelszó")
		visible: _radioPlain.checked
		echoMode: TextInput.Password
		inputMethodHints: Qt.ImhSensitiveData
		validator: RegExpValidator { regExp: /.+/ }
		errorText: qsTr("Meg kell adni egy jelszót")
		trailingContent: Qaterial.TextFieldPasswordButton { textField: _password }
	}

	QFormTextField {
		id: _password2
		width: parent.width
		visible: _radioPlain.checked
		title: qsTr("Jelszó mégegyszer")
		enabled: _password.echoMode == TextInput.Password
		echoMode: TextInput.Password
		inputMethodHints: Qt.ImhSensitiveData
		error: enabled && text !== _password.text
		errorText: qsTr("Nem egyeznek meg a jelszavak")
	}


	QButton
	{
		id: button
		anchors.horizontalCenter: parent.horizontalCenter
		text: qsTr("Regisztráció")
		icon.source: Qaterial.Icons.disc
		enabled: _code.acceptableInput &&
				 (_radioGoogle.checked ||
				  (_familyName.acceptableInput && _password.acceptableInput &&
				   (_password.echoMode == TextInput.Normal || _password.text == _password2.text)))
		onClicked:
		{
			if (_radioPlain.checked)
				Client.registrationPlain({
											 code: _code.text,
											 username: _username.text,
											 familyName: _familyName.text,
											 givenName: _givenName.text,
											 password: _password.text
										 })
			else if (_radioGoogle.checked)
				Client.registrationGoogle(_code.text)

		}
	}

	StackView.onActivated: _code.forceActiveFocus()
}
