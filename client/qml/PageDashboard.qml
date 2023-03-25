import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import "JScript.js" as JS

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

	property TeacherGroupList groupListTeacher: Client.cache("teacherGroupList")
	property StudentGroupList groupListStudent: Client.cache("studentGroupList")

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
			id: groupsTeacherGrid
			anchors.horizontalCenter: parent.horizontalCenter

			visible: Client.server && (Client.server.user.roles & Credential.Teacher)

			Repeater {
				model: SortFilterProxyModel {
					sourceModel: groupListTeacher
					sorters: [
						RoleSorter {
							roleName: "active"
							priority: 1
							sortOrder: Qt.DescendingOrder
						},
						StringSorter {
							roleName: "name"
							sortOrder: Qt.AscendingOrder
						}

					]
				}

				QDashboardButton {
					property TeacherGroup group: model.qtObject
					text: group ? group.fullName : ""
					icon.source: Qaterial.Icons.group
					bgColor: Qaterial.Colors.green700
					outlined: !group.active
					flat: !group.active
					onClicked: Client.stackPushPage("PageTeacherGroup.qml", {
														group: group
													})
				}
			}

			QDashboardButton {
				action: actionGroupAdd
				highlighted: false
				outlined: true
				flat: true
				textColor: Qaterial.Colors.green400
			}
		}

		Qaterial.HorizontalLineSeparator {
			anchors.horizontalCenter: parent.horizontalCenter
			visible: groupsStudentGrid.visible && groupsStudentGrid.visible
			width: Math.max(groupsTeacherGrid.width, groupsStudentGrid.width)*0.75
		}

		QDashboardGrid {
			id: groupsStudentGrid
			anchors.horizontalCenter: parent.horizontalCenter

			visible: groupListStudent.count

			Repeater {
				model: SortFilterProxyModel {
					sourceModel: groupListStudent
					sorters: [
						StringSorter {
							roleName: "name"
							sortOrder: Qt.AscendingOrder
						}
					]
				}

				QDashboardButton {
					property StudentGroup group: model.qtObject
					text: group ? group.name : ""
					icon.source: Qaterial.Icons.group
					highlighted: true
				}
			}
		}

		Qaterial.HorizontalLineSeparator {
			anchors.horizontalCenter: parent.horizontalCenter
			visible: groupsStudentGrid.visible || groupsTeacherGrid.visible
			width: Math.max(groupsTeacherGrid.width, groupsStudentGrid.width)*0.75
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


	Action {
		id: actionGroupAdd
		text: qsTr("Létrehozás")
		icon.source: Qaterial.Icons.plus
		enabled: Client.server && (Client.server.user.roles & Credential.Teacher)
		onTriggered: {
			Qaterial.DialogManager.showTextFieldDialog({
														   textTitle: qsTr("Csoport neve"),
														   title: qsTr("Új csoport létrehozása"),
														   standardButtons: Dialog.Cancel | Dialog.Ok,
														   onAccepted: function(_text, _noerror) {
															   if (_noerror && _text.length)
																   Client.send(WebSocket.ApiTeacher, "group/create", { name: _text })
															   .done(function(r){ Client.reloadCache("teacherGroupList") })
															   .fail(JS.failMessage("Létrehozás sikertelen"))
														   }
													   })
		}
	}

	StackView.onActivated: {
		if (Client.server && (Client.server.user.roles & Credential.Teacher)) {
			Client.reloadCache("teacherGroupList")
		}
	}

}
