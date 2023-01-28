import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

QPage {
	id: control

	closeQuestion: qsTr("Biztosan lezárod a kapcsolatot a szerverrel?")

	onPageClose: function() {
		if (Client.server && Client.server.user.loginState == User.LoggedIn)
			Client.webSocket.close()
	}


	/*stackPopFunction: function() {
		if (swipeView.currentIndex > 0) {
			swipeView.setCurrentIndex(0)
			return false
		}

		if (_login.registrationMode) {
			_login.registrationMode = false
			return false
		}

		return true
	}*/

	title: Client.server ? Client.server.serverName : ""
	subtitle: Client.server ? Client.server.user.fullName : ""

	appBar.backButtonVisible: true
	appBar.rightComponent: _cmpUser //swipeView.currentIndex == 1 ? _cmpRank : _cmpUser
	appBar.rightPadding: Qaterial.Style.horizontalPadding

	Component {
		id: _cmpUser
		UserButton { }
	}

	QScrollable {
		id: _content
		anchors.fill: parent

		Qaterial.LabelHeadline3 {
			width: parent.width
			text: "user name"
		}

		Row {
			UserImage {
				user: Client.server ? Client.server.user : null
				iconColor: Qaterial.Style.colorTheme.primaryText
				width: Qaterial.Style.pixelSize*2.5
				height: Qaterial.Style.pixelSize*2.5
				pictureEnabled: false
			}

			Qaterial.LabelHeadline6 {

				text: Client.server ? Client.server.user.rank.name : ""
			}
		}


		QDashboardGrid {
			id: groupsGrid
			anchors.horizontalCenter: parent.horizontalCenter

			Repeater {
				model: 8

				QDashboardButton {
					text: "Csoport hosszú névvel, mi lesz vele vajon"+index
					icon.source: Qaterial.Icons.group
				}
			}

			QDashboardButton {
				visible: Client.server && (Client.server.user.roles & Credential.Teacher)
				text: qsTr("Létrehozás")
				icon.source: Qaterial.Icons.plus
				highlighted: false
				outlined: true
				//backgroundColor: Client.Utils.colorSetAlpha("black", 0.7)
				flat: true

				textColor: Qaterial.Colors.green500
			}
		}

		Qaterial.HorizontalLineSeparator {
			anchors.horizontalCenter: parent.horizontalCenter
			width: groupsGrid.width*0.75
		}


		QDashboardGrid {
			id: funcGrid
			anchors.horizontalCenter: parent.horizontalCenter

			QDashboardButton {
				visible: Client.server && ((Client.server.user.roles & Credential.Teacher) || (Client.server.user.roles & Credential.Admin))
				text: qsTr("Pályaszerkesztő")
				icon.source: Qaterial.Icons.map
				highlighted: false
				outlined: true
				flat: true

				textColor: Qaterial.Colors.amber500
			}

			QDashboardButton {
				visible: Client.server && (Client.server.user.roles & Credential.Admin)
				text: qsTr("Osztályok és felhasználók")
				icon.source: Qaterial.Icons.cog
				highlighted: false
				outlined: true
				flat: true

				textColor: Qaterial.Colors.red500

				onClicked: Client.stackPushPage("PageAdminUsers.qml")
			}

		}

	}

}
