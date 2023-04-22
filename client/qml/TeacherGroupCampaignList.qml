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
	property TeacherMapHandler mapHandler: null
	property alias view: view

	property alias actionCampaignAdd: actionCampaignAdd

	QListView {
		id: view

		currentIndex: -1
		autoSelectChange: true

		height: parent.height
		width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
		anchors.horizontalCenter: parent.horizontalCenter

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
			iconSource: {
				if (!campaign)
					return ""
				switch (campaign.state) {
				case Campaign.Finished:
					return Qaterial.Icons.checkBold
				case Campaign.Running:
					return Qaterial.Icons.play
				default:
					return Qaterial.Icons.account
				}
			}


			readonly property string _campaignName: campaign ? campaign.readableName : ""

			text: _campaignName
			secondaryText: campaign ? campaign.taskList.length + " size" : ""

			onClicked: if (!view.selectEnabled)
						   Client.stackPushPage("PageTeacherCampaign.qml", {
													group: control.group,
													campaign: campaign,
													mapHandler: control.mapHandler
												})
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
		enabled: group
		text: qsTr("Új hadjárat")
		icon.source: Qaterial.Icons.accountPlus
		onTriggered: {
			Client.send(WebSocket.ApiTeacher, "group/%1/campaign/create".arg(group.groupid))
			.done(function(r){
				group.reloadAndCall(function() {
					var o = Client.findOlmObject(group.campaignList, "campaignid", r.id)
					if (o)
						Client.stackPushPage("PageTeacherCampaign.qml", {
												 group: control.group,
												 campaign: o
											 })
				})
			})
			.fail(JS.failMessage(qsTr("Hadjárat létrehozása sikertelen")))
		}
	}
}
