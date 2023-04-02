import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import "./JScript.js" as JS

Qaterial.Page
{
	id: control

	implicitWidth: 200
	implicitHeight: 200

	background: Rectangle { color: "transparent" }

	property TeacherGroup group: null
	property alias view: view

	property alias actionCampaignAdd: actionCampaignAdd

	QListView {
		id: view

		currentIndex: -1
		anchors.fill: parent
		autoSelectChange: true

		refreshProgressVisible: Client.webSocket.pending
		refreshEnabled: true
		onRefreshRequest: group.reload()

		model: SortFilterProxyModel {
			sourceModel: group ? group.campaignList : null

			sorters: [
				RoleSorter {
					roleName: "finished"
					sortOrder: Qt.AscendingOrder
					priority: 3
				},
				RoleSorter {
					roleName: "started"
					sortOrder: Qt.DescendingOrder
					priority: 2
				},
				RoleSorter {
					roleName: "startTime"
					sortOrder: Qt.AscendingOrder
					priority: 1
				}
			]
		}

		delegate: QItemDelegate {
			property Campaign campaign: model.qtObject
			selectableObject: campaign

			highlighted: ListView.isCurrentItem
			iconSource: Qaterial.Icons.account

			text: campaign ? campaign.campaignid : ""
			secondaryText: campaign ? campaign.taskList.length + " size" : ""

			/*onClicked: if (!view.selectEnabled)
						   Client.stackPushPage("AdminUserEdit.qml", {
													user: user
												})*/
		}
	}

	Qaterial.Banner
	{
		anchors.top: parent.top
		width: parent.width
		drawSeparator: true
		text: qsTr("Még egyetlen hadjárat sincsen felvéve. Hozz létre egyet.")
		iconSource: Qaterial.Icons.account
		fillIcon: false
		outlinedIcon: true
		highlightedIcon: true

		action1: qsTr("Létrehozás")

		onAction1Clicked: actionCampaignAdd.trigger()

		enabled: group
		visible: group && !group.campaignList.length
	}

	QFabButton {
		visible: view.visible
		action: actionCampaignAdd
	}


	Action {
		id: actionCampaignAdd
		text: qsTr("Új hadjárat")
		icon.source: Qaterial.Icons.accountPlus
		/*onTriggered: Client.stackPushPage("AdminUserEdit.qml", {
											  classid: _user.classid
										  })*/
	}



}
