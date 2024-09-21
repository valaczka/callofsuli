import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

QFormColumn {
	id: root

	property int api: HttpConnection.ApiUser
	property string path: "password"

	spacing: 3

	QFormTextField {
		id: _passwordOld
		width: parent.width
		title: qsTr("Régi jelszó")
		echoMode: TextInput.Password
		inputMethodHints: Qt.ImhSensitiveData
		validator: RegularExpressionValidator { regularExpression: /.+/ }
		trailingContent: Qaterial.TextFieldButtonContainer
		{
			Qaterial.TextFieldPasswordButton { }
		}
		watchModification: false
	}


	QFormTextField {
		id: _password
		width: parent.width
		title: qsTr("Új jelszó")
		echoMode: TextInput.Password
		inputMethodHints: Qt.ImhSensitiveData
		validator: RegularExpressionValidator { regularExpression: /.+/ }
		trailingContent: Qaterial.TextFieldButtonContainer
		{
			Qaterial.TextFieldPasswordButton { }
		}
		watchModification: false

		readonly property bool _hasAcceptableInput: acceptableInput && (echoMode == TextInput.Normal || text == _password2.text)
	}

	QFormTextField {
		id: _password2
		width: parent.width
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
		text: qsTr("Új jelszó beállítása")
		icon.source: Qaterial.Icons.contentSave
		enabled: _passwordOld.acceptableInput && _password._hasAcceptableInput
		onClicked: {
			root.enabled = false
			Client.send(api, path, {
							oldPassword: _passwordOld.text,
							password: _password.text
						})
			.done(root, function(r){
				_passwordOld.text = ""
				_password.text = ""
				_password2.text = ""
				root.enabled = true
				Client.snack(qsTr("Jelszóváltoztatás sikeres"))
			})
			.fail(root, function(err) {
				Client.messageWarning(err, qsTr("Jelszóváltoztatás sikertelen"))
				root.enabled = true
			})
			.error(root, function(err) {
				root.enabled = true
			})
		}
	}
}
