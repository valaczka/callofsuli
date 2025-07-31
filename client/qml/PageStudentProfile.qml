import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPageGradient {
	id: root

	title: qsTr("Profil")

	appBar.backButtonVisible: true

	progressBarEnabled: true


	property User user: Client.server.user
	property var userData: null

	property bool _isFirst: true

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

		refreshEnabled: true

		onRefreshRequest: reload()

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
				nameEditable: Client.server && Client.server.config.nameUpdateEnabled === true
				pictureEditable: Client.server && Client.server.config.pictureUpdateEditable === true

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

			header: QExpandableHeader {
				text: qsTr("Jelszó megváltoztatása")
				icon: Qaterial.Icons.security
				expandable: _expPassword
			}

			delegate: UserInfoPassword {
				id: _formPassword
			}
		}

		Qaterial.Expandable {
			id: _expNotification
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			visible: user && user.oauth != ""

			expanded: false

			header: QExpandableHeader {
				text: qsTr("Emailes értesítések")
				icon: Qaterial.Icons.emailAlertOutline
				expandable: _expNotification
			}

			delegate: UserInfoNotification {
				id: _formNotification
				width: _expNotification.width
				username: userData ? userData.username : ""
			}
		}

		UserInfoCounter {
			id: _counter
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter
			userLogList: _userLog
		}

		UserInfoLog {
			id: _log
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter
			userLogList: _userLog
		}


	}


	onUserChanged: {
		if (user)
			userData = user.toVariantMap()
		else
			userData = null
	}


	UserLogListImpl {
		id: _userLog
		username: userData ? userData.username: ""
	}


	StackView.onActivated: {
		if (_isFirst) {
			reload()
			_isFirst = false
		}
	}

	function reload() {
		if (!userData || !userData.username)
			return

		Client.send(HttpConnection.ApiGeneral, "user/%1".arg(userData.username))
		.done(root, function(r){
			userData = Client.userToMap(r)
			_userLog.reload()
		})
		.fail(root, JS.failMessage("Letöltés sikertelen"))
	}
}
