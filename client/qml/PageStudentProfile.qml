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
	property alias _tour: _tour

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
					root._tour.list[1].target = _form.tfNickName

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
				id: _headerNotification
				text: qsTr("Emailes értesítések")
				icon: Qaterial.Icons.emailAlertOutline
				expandable: _expNotification

				Component.onCompleted: root._tour.list[0].target = button
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


	SpotlightCoachTour {
		id: _tour

		page: "studentprofile"

		basePage: root

		list: [
			{ target: null, title: qsTr("E-mailes értesítések"), text: qsTr("Itt tudsz kérni értesítéseket e-mailben")},
			{ id: 1, target: null, title: qsTr("Becenév"), text: qsTr("Itt tudsz beállítani magadnak becenevet") },
		]
	}


	StackView.onActivated: {
		Client.contextHelper.setCurrentContext(ContextHelperData.ContextStudentProfile)
		_tour.start()

		if (_isFirst) {
			reload()
			_isFirst = false
		}
	}

	StackView.onDeactivating: {
		Client.contextHelper.unsetContext(ContextHelperData.ContextStudentProfile)
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
