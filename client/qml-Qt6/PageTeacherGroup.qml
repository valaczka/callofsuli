import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPage {
	id: control

	title: group ? group.fullName : qsTr("Csoport")
	subtitle: Client.server ? Client.server.serverName : ""

	property TeacherGroup group: null
	property TeacherMapHandler mapHandler: null

	appBar.backButtonVisible: true
	appBar.rightComponent: Qaterial.AppBarButton
	{
		icon.source: Qaterial.Icons.dotsVertical
		onClicked: _menu.open()

		QMenu {
			id: _menu

			QMenuItem { action: _actionGroupRename }
			QMenuItem { action: _actionGroupRemove }
			QMenuItem { action: _actionUserEdit }
		}
	}


	QScrollable {
		anchors.fill: parent
		contentCentered: true

		Qaterial.LabelHeadline3 {
			anchors.horizontalCenter: parent.horizontalCenter
			topPadding: 30
			width: Math.min(parent.width-100, Qaterial.Style.maxContainerSize)
			horizontalAlignment: Qt.AlignHCenter
			text: group ? group.fullName : ""
			wrapMode: Text.Wrap
			maximumLineCount: 2
			elide: Text.ElideRight
		}

		QDashboardGrid {
			anchors.horizontalCenter: parent.horizontalCenter

			QDashboardButton {
				text: qsTr("Kihívások")
				icon.source: Qaterial.Icons.trophyVariantOutline
				onClicked: {
					Client.stackPushPage("PageTeacherGroupCampaign.qml", {
											 group: group,
											 mapHandler: mapHandler
										 })
				}
			}

			QDashboardButton {
				text: qsTr("Dolgozatok")
				icon.source: Qaterial.Icons.paperCutVertical
				onClicked: {
					Client.stackPushPage("PageTeacherGroupExam.qml", {
											 group: group,
											 mapHandler: mapHandler
										 })
				}
			}

			QDashboardButton {
				action: _actionGroupRename
				highlighted: false
				outlined: true
				flat: true
				textColor: Qaterial.Colors.green400
			}

			QDashboardButton {
				action: _actionUserEdit
				highlighted: false
				outlined: true
				flat: true
				textColor: Qaterial.Colors.green400
			}

			QDashboardButton {
				action: _actionGroupRemove
				highlighted: false
				outlined: true
				flat: true
				textColor: Qaterial.Colors.red400
			}
		}
	}



	Action {
		id: _actionGroupRemove
		text: qsTr("Törlés")
		enabled: group
		icon.source: Qaterial.Icons.delete_
		onTriggered: {
			JS.questionDialog(
						{
							onAccepted: function()
							{
								Client.send(HttpConnection.ApiTeacher, "group/%1/delete".arg(group.groupid))
								.done(control, function(r){
									Client.reloadCache("teacherGroupList")
									Client.stackPop(control)
								})
								.fail(control, JS.failMessage("Törlés sikertelen"))
							},
							text: qsTr("Biztosan törlöd a csoportot?"),
							title: group.name,
							iconSource: Qaterial.Icons.closeCircle
						})
		}
	}


	Action {
		id: _actionGroupRename
		text: qsTr("Átnevezés")
		enabled: group
		icon.source: Qaterial.Icons.accountMultipleCheck
		onTriggered: {
			Qaterial.DialogManager.showTextFieldDialog({
														   textTitle: qsTr("Csoport neve"),
														   title: qsTr("Csoport átnevezése"),
														   text: group.name,
														   standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok,
														   onAccepted: function(_text, _noerror) {
															   if (_noerror && _text.length)
																   Client.send(HttpConnection.ApiTeacher, "group/%1/update".arg(group.groupid),
																			   {
																				   name: _text
																			   })
															   .done(control, function(r){
																   Client.reloadCache("teacherGroupList")
															   })
															   .fail(control, JS.failMessage("Átnevezés sikertelen"))
														   }
													   })
		}
	}


	Action {
		id: _actionUserEdit
		icon.source: Qaterial.Icons.accountEdit
		text: qsTr("Résztvevők")
		onTriggered: {
			Client.stackPushPage("PageTeacherGroupEdit.qml", {
									 group: group
								 })

		}
	}

	Component.onCompleted: group.reload()
}
