import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import "JScript.js" as JS

QPage {
	id: root

	property bool _closeEnabled: false

	stackPopFunction: function() {
		if (_closeEnabled || (Client.server && Client.server.user.loginState != User.LoggedIn))
			return true

		JS.questionDialog(
					{
						onAccepted: function()
						{
							_closeEnabled = true
							if (Client.server && Client.server.user.loginState == User.LoggedIn)
								Client.webSocket.close()
							else
								Client.stackPop(root)
						},
						text: qsTr("Biztosan lezárod a kapcsolatot a szerverrel?"),
						title: qsTr("Kilépés"),
						iconSource: Qaterial.Icons.closeCircle
					})


		return false
	}

	title: Client.server ? Client.server.serverName : ""
	subtitle: user ? user.fullName : ""

	appBar.backButtonVisible: true
	appBar.rightComponent: _cmpUser //swipeView.currentIndex == 1 ? _cmpRank : _cmpUser
	appBar.rightPadding: Qaterial.Style.horizontalPadding

	property User user: Client.server ? Client.server.user : null
	property TeacherGroupList groupListTeacher: Client.cache("teacherGroupList")
	property StudentGroupList groupListStudent: Client.cache("studentGroupList")
	property TeacherMapHandler mapHandler: TeacherMapHandler { }

	property bool _firstRun: true

	Component {
		id: _cmpUser
		UserButton { }
	}


	QScrollable {
		anchors.fill: parent
		contentCentered: true

		Qaterial.LabelHeadline3 {
			anchors.horizontalCenter: parent.horizontalCenter
			topPadding: 30
			width: Math.min(parent.width-100, Qaterial.Style.maxContainerSize)
			horizontalAlignment: Qt.AlignHCenter
			text: user ? user.fullName : ""
			wrapMode: Text.Wrap
			maximumLineCount: 2
			elide: Text.ElideRight
		}

		Row {
			spacing: 10
			anchors.horizontalCenter: parent.horizontalCenter

			UserImage {
				user: root.user
				iconColor: Qaterial.Style.colorTheme.primaryText
				width: Qaterial.Style.pixelSize*2.5
				height: Qaterial.Style.pixelSize*2.5
				pictureEnabled: false
				anchors.verticalCenter: parent.verticalCenter
			}

			Column {
				anchors.verticalCenter: parent.verticalCenter
				Qaterial.LabelHeadline6 {
					anchors.left: parent.left
					text: user ? user.rank.name : ""
				}
				Qaterial.LabelBody2 {
					anchors.left: parent.left
					text: user && user.rank.sublevel > 0 ? qsTr("level %1").arg(user.rank.sublevel) : ""
					visible: text != ""
				}
			}
		}



		// ---- TEACHER GROUPS

		QDashboardGrid {
			id: groupsTeacherGrid
			anchors.horizontalCenter: parent.horizontalCenter

			visible: Client.server && (Client.server.user.roles & Credential.Teacher)
			contentItems: showPlaceholders ? 2 : groupListTeacher.lenght+1

			readonly property bool showPlaceholders: groupListTeacher.count === 0 && _firstRun

			SortFilterProxyModel {
				id: _sortedTeacherGroups
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

			Repeater {
				model: groupsTeacherGrid.showPlaceholders ? 2 : _sortedTeacherGroups

				delegate: groupsTeacherGrid.showPlaceholders ? _cmpPlaceholder : _cmpGroupButton
			}

			Component {
				id: _cmpGroupButton

				QDashboardButton {
					property TeacherGroup group: model && model.qtObject ? model.qtObject : null
					text: group ? group.fullName : ""
					icon.source: Qaterial.Icons.group
					bgColor: Qaterial.Colors.green700
					outlined: group && !group.active
					flat: group && !group.active
					onClicked: Client.stackPushPage("PageTeacherGroup.qml", {
														group: group,
														mapHandler: root.mapHandler
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


		Component {
			id: _cmpPlaceholder

			QPlaceholderItem {
				fixedWidth: groupsTeacherGrid.buttonSize
				fixedHeight: groupsTeacherGrid.buttonSize
				rectangleRadius: 5
			}
		}



		Qaterial.HorizontalLineSeparator {
			anchors.horizontalCenter: parent.horizontalCenter
			visible: groupsStudentGrid.visible && groupsStudentGrid.visible
			width: Math.max(groupsTeacherGrid.width, groupsStudentGrid.width)*0.75
		}



		// ------- STUDENT GROUPS

		QDashboardGrid {
			id: groupsStudentGrid
			anchors.horizontalCenter: parent.horizontalCenter

			visible: groupListStudent.count
			contentItems: groupListStudent.count

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





		// ------- FUNCTIONS

		QDashboardGrid {
			id: funcGrid
			anchors.horizontalCenter: parent.horizontalCenter

			QDashboardButton {
				visible: Client.server && ((Client.server.user.roles & Credential.Teacher) || (Client.server.user.roles & Credential.Admin))
				text: qsTr("Pályák")
				icon.source: Qaterial.Icons.map
				highlighted: false
				outlined: true
				flat: true

				textColor: Qaterial.Colors.amber500

				onClicked: Client.stackPushPage("PageTeacherMaps.qml", {
													handler: mapHandler
												})
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
															   .done(function(r){
																   Client.reloadCache("teacherGroupList", function() {
																	   var d = Client.findCacheObject("teacherGroupList", r.id)
																	   if (d)
																		   Client.stackPushPage("PageTeacherGroup.qml", {
																									group: d
																								})

																   })
															   })
															   .fail(JS.failMessage("Létrehozás sikertelen"))
														   }
													   })
		}
	}


	StackView.onActivated: {
		if (Client.server) {
			Client.reloadCache("teacherGroupList", function() {
				_firstRun = false
			})
		}

		mapHandler.reload()
	}
}
