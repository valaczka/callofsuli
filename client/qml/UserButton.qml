import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

Qaterial.SquareButton {
	id: control

	visible: Client.server && Client.server.user.loginState == User.LoggedIn

	ToolTip.text: Client.server ? Client.server.user.fullName : ""

	property real contentSize: Math.min(width,height)*0.85

	contentItem: Item{
		UserImage {
			user: Client.server ? Client.server.user : null
			iconColor: Qaterial.Style.colorTheme.primaryText
			width: control.contentSize
			height: control.contentSize
			anchors.centerIn: parent
		}
	}

	Action {
		id: actionLogout
		text: qsTr("Kijelentkezés")
		icon.source: Qaterial.Icons.logoutVariant
		onTriggered: Client.logout()
	}

	QMenu {
		id: menu
		QMenuItem { action: actionLogout }
	}


	onClicked: menu.open()
}
