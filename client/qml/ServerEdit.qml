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

	property Server server: null

	title: server ? server.serverName : qsTr("Új szerver")

	appBar.rightComponent: Qaterial.AppBarButton
	{
		visible: server
		ToolTip.text: qsTr("Szerver törlése")
		icon.source: Qaterial.Icons.delete_
		onClicked: JS.questionDialog(
					   {
						   onAccepted: function()
						   {
							   if (Client.serverDelete(server)) {
								   _form.modified = false
								   Client.stackPop(control)
							   }
						   },
						   text: qsTr("Biztosan törlöd a szervert?"),
						   title: server.serverName,
						   iconSource: Qaterial.Icons.closeCircle
					   })

	}

	QScrollable {
		anchors.fill: parent

	QFormColumn {
		id: _form

		title: qsTr("Szerver adatai")

		QFormTextField {
			id: _host
			title: qsTr("Host")
			width: parent.width
			helperText: qsTr("A szerver IP címe, vagy domain neve")
			placeholderText: qsTr("192.168.2.1, www.callofsuli.hu,...stb.")
			validator: RegularExpressionValidator { regularExpression: /.+/ }
			errorText: qsTr("A szerver IP címe szükséges")
			leadingIconSource: Qaterial.Icons.ipNetwork
			trailingContent: Qaterial.TextFieldButtonContainer
			{
				Qaterial.TextFieldAlertIcon {  }
				Qaterial.TextFieldClearButton { }
			}
		}

		QFormTextField {
			id: _port
			title: qsTr("Port")
			width: parent.width
			helperText: qsTr("A szerver portja")
			placeholderText: "10101"
			inputMethodHints: Qt.ImhDigitsOnly
			validator: IntValidator { id: _portValidator; bottom: 1; top: 65535 }
			errorText: qsTr("A port szükséges, %1-%2 között kell lennie").arg(_portValidator.bottom).arg(_portValidator.top)
			leadingIconSource: Qaterial.Icons.numeric10BoxMultipleOutline
			trailingContent: Qaterial.TextFieldButtonContainer
			{
				Qaterial.TextFieldAlertIcon { }
			}
		}

		QFormCheckButton
		{
			id: _ssl
			text: qsTr("SSL kapcsolat")
		}

		Qaterial.LabelCaption
		{
			width: parent.width
			wrapMode: Label.Wrap
			text: qsTr("Az applikáció induláskor a kiválasztott szerverhez megpróbál csatlakozni.")
			topPadding: 10
		}

		QFormSwitchButton
		{
			id: _auto
			text: qsTr("Automatikus csatlakozás")
		}

		QButton
		{
			anchors.horizontalCenter: parent.horizontalCenter
			text: qsTr("Mentés")
			icon.source: Qaterial.Icons.contentSave
			enabled: _host.acceptableInput && _port.acceptableInput && _form.modified
			onClicked:
			{
				if (!server) {
					server = Client.serverAdd()
					if (server)
						server.serverName = qsTr("-- új szerver --")
				}

				if (server) {
					server.url = (_ssl.checked ? "https://" : "http://")+_host.text+":"+_port.text
					if (_auto.checked)
						Client.serverSetAutoConnect(server)
					else
						server.autoConnect = false

					_form.modified = false
					Client.stackPop(control)
				}
			}
		}
	}

	}

	Component.onCompleted: if (server) {
						 _host.text = server.host()
						 _port.text = server.port()
						 _ssl.checked = server.ssl()
						 _auto.checked = server.autoConnect
					 }


	StackView.onActivated: _host.forceActiveFocus()
}
