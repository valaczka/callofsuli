import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
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
		validator: RegularExpressionValidator { regularExpression: /.+/ }
		errorText: qsTr("Meg kell adni a hitelesítő kódot")
		leadingIconSource: Qaterial.Icons.keyOutline
		inputMethodHints: Qt.ImhSensitiveData
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
		text: qsTr("Válaszd ki, hogyan szeretnél regisztrálni:")
		topPadding: 10
	}

	QFormRadioButton {
		id: _radioPlain
		text: qsTr("Regisztráció felhasználónévvel, jelszóval")
		enabled: Client.server && (Client.server.config.oauth2RegistrationForced === undefined || !Client.server.config.oauth2RegistrationForced)
	}

	QFormRadioButton {
		id: _radioGoogle
		text: qsTr("Regisztráció Google fiókkal")
		checked: true
		visible: Client.server && Client.server.config.oauthProviders !== undefined && Client.server.config.oauthProviders.includes("google")
	}

	QFormRadioButton {
		id: _radioMicrosoft
		text: qsTr("Regisztráció Microsoft fiókkal")
		visible: Client.server && Client.server.config.oauthProviders !== undefined && Client.server.config.oauthProviders.includes("microsoft")
	}

	QFormTextField {
		id: _username
		title: qsTr("Felhasználónév")
		width: parent.width
		visible: _radioPlain.checked
		validator: RegularExpressionValidator { regularExpression: /.+/ }
		errorText: qsTr("Meg kell adni egy felhasználónevet")
		inputMethodHints: Qt.ImhNoPredictiveText
		//leadingIconSource: Qaterial.Icons.remoteDesktop
		trailingContent: Qaterial.TextFieldButtonContainer
		{
			Qaterial.TextFieldAlertIcon {  }
			Qaterial.TextFieldClearButton { }
		}
	}


	QFormTextField {
		id: _familyName
		title: qsTr("Vezetéknév")
		width: parent.width
		visible: _radioPlain.checked
		validator: RegularExpressionValidator { regularExpression: /.+/ }
		errorText: qsTr("Meg kell adni a vezetéknevet")
		//leadingIconSource: Qaterial.Icons.remoteDesktop
		trailingContent: Qaterial.TextFieldButtonContainer
		{
			Qaterial.TextFieldAlertIcon { }
			Qaterial.TextFieldClearButton {  }
		}
	}

	QFormTextField {
		id: _givenName
		title: qsTr("Keresztnév")
		width: parent.width
		visible: _radioPlain.checked
		//leadingIconSource: Qaterial.Icons.remoteDesktop
		trailingContent: Qaterial.TextFieldButtonContainer
		{
			Qaterial.TextFieldAlertIcon { }
			Qaterial.TextFieldClearButton {  }
		}
	}

	QFormTextField {
		id: _password
		width: parent.width
		title: qsTr("Jelszó")
		visible: _radioPlain.checked
		echoMode: TextInput.Password
		inputMethodHints: Qt.ImhSensitiveData
		validator: RegularExpressionValidator { regularExpression: /.+/ }
		errorText: qsTr("Meg kell adni egy jelszót")
		trailingContent: Qaterial.TextFieldButtonContainer
		{
			Qaterial.TextFieldPasswordButton { }
		}
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
		icon.source: Qaterial.Icons.loginVariant
		enabled: _code.acceptableInput &&
				 (_radioGoogle.checked || _radioMicrosoft.checked ||
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
				Client.registrationOAuth2("google", _code.text)
			else if (_radioMicrosoft.checked)
				Client.registrationOAuth2("microsoft", _code.text)

		}
	}

	StackView.onActivated: _code.forceActiveFocus()
}
