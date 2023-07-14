import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import "./JScript.js" as JS

Item
{
	id: control

	property TeacherGroup group: null
	property TeacherMapHandler mapHandler: null
	property alias view: view

	property alias actionCampaignAdd: actionCampaignAdd

	QScrollable {
		anchors.fill: parent
		topPadding: 0
		leftPadding: 0
		bottomPadding: 0
		rightPadding: 0

		refreshEnabled: true
		onRefreshRequest: group.reload()

		QListView {
			id: view

			currentIndex: -1
			autoSelectChange: false

			height: contentHeight
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			boundsBehavior: Flickable.StopAtBounds

			refreshProgressVisible: Client.webSocket.pending

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
				//selectableObject: campaign

				highlighted: ListView.isCurrentItem
				iconSource: {
					if (!campaign)
						return ""
					switch (campaign.state) {
					case Campaign.Finished:
						return Qaterial.Icons.checkBold
					case Campaign.Running:
						return Qaterial.Icons.playCircle
					default:
						return Qaterial.Icons.account
					}
				}

				iconColor: {
					if (!campaign)
						return Qaterial.Style.disabledTextColor()
					switch (campaign.state) {
					case Campaign.Finished:
						return Qaterial.Style.iconColor()
					case Campaign.Running:
						return Qaterial.Colors.green400
					default:
						return Qaterial.Style.disabledTextColor()
					}
				}

				textColor: iconColor
				secondaryTextColor: !campaign || campaign.state == Campaign.Finished ?
										Qaterial.Style.disabledTextColor() : Qaterial.Style.colorTheme.secondaryText


				readonly property string _campaignName: campaign ? campaign.readableName : ""

				text: _campaignName
				secondaryText: {
					if (!campaign)
						return ""

					if (campaign.startTime.getTime()) {
						return campaign.startTime.toLocaleString(Qt.locale(), "yyyy. MMM d. HH:mm – ")
								+ (campaign.endTime.getTime() ? campaign.endTime.toLocaleString(Qt.locale(), "yyyy. MMM d. HH:mm") : "")
					}

					return ""
				}

				onClicked: Client.stackPushPage("PageTeacherCampaign.qml", {
													group: control.group,
													campaign: campaign,
													mapHandler: control.mapHandler
												})
			}
		}

	}

	Qaterial.Banner
	{
		anchors.top: parent.top
		width: parent.width
		drawSeparator: true
		text: qsTr("Még egyetlen hadjárat sincsen felvéve. Hozz létre egyet.")
		iconSource: Qaterial.Icons.trophyVariantOutline
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
