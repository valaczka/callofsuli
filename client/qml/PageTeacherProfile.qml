import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import "JScript.js" as JS

QPage {
	id: root

	title: Client.server ? Client.server.serverName : ""
	subtitle: user ? user.fullName : ""


	property User user: Client.server.user
	property var userData: null

	appBar.backButtonVisible: true
	appBar.rightComponent: Qaterial.AppBarButton {
		icon.source: Qaterial.Icons.logoutVariant
		ToolTip.text: qsTr("Kijelentkezés")
		onClicked: {
			JS.questionDialog({
								  onAccepted: function()
								  {
									  Client.logout()
								  },
								  text: qsTr("Biztosan kijelentkezel?"),
								  iconSource: Qaterial.Icons.logoutVariant,
								  title: Client.server ? Client.server.serverName : ""
							  })
		}
	}

	QScrollable {
		anchors.fill: parent
		spacing: 15
		contentCentered: true

		UserInfoHeader {
			id: _header
			width: parent.width
			userData: root.userData

			topPadding: root.paddingTop
		}

		Qaterial.Expandable {
			id: _expForm
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			expanded: true

			header: QExpandableHeader {
				text: qsTr("Felhasználói adatok")
				icon: Qaterial.Icons.accountOutline
				expandable: _expForm
			}

			delegate: UserInfoForm {
				id: _form
				width: _expForm.width
				editable: true
				nameEditable: true
				pictureEditable: true

				Component.onCompleted: {
					if (root.userData)
						loadData(root.userData)
				}

				Connections {
					target: root

					function onUserDataChanged() {
						_form.loadData(root.userData)
					}
				}
			}
		}


		Qaterial.Expandable {
			id: _expPassword
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			visible: user && user.oauth == ""

			expanded: false

			property bool editable: false

			header: QExpandableHeader {
				text: qsTr("Jelszó megváltoztatása")
				icon: Qaterial.Icons.security
				expandable: _expPassword
			}

			delegate: UserInfoPassword {
				id: _formPassword
			}
		}


	}


	onUserChanged: {
		if (user)
			userData = user.toVariantMap()
		else
			userData = null
	}


	StackView.onActivated: { reload() }

	function reload() {
		if (!userData || !userData.username)
			return

		Client.send(HttpConnection.ApiGeneral, "user/%1".arg(userData.username))
		.done(root, function(r){
			userData = Client.userToMap(r)
		})
		.fail(root, JS.failMessage("Letöltés sikertelen"))
	}

}


