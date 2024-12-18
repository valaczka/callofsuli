import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import SortFilterProxyModel
import "./JScript.js" as JS

Item
{
	id: control

	property TeacherGroup group: null
	property TeacherMapHandler mapHandler: null

	property alias actionCampaignAdd: actionCampaignAdd

	property var stackPopFunction: function() {
		if (view.selectEnabled) {
			view.unselectAll()
			return false
		}

		return true
	}

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

				iconColorBase: {
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
		text: qsTr("Még egyetlen kihívás sincsen felvéve. Hozz létre egyet.")
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
		text: qsTr("Új kihívás")
		icon.source: Qaterial.Icons.plus
		onTriggered: {
			Client.send(HttpConnection.ApiTeacher, "group/%1/campaign/create".arg(group.groupid))
			.done(control, function(r){
				group.reloadAndCall(control, function() {
					var o = Client.findOlmObject(group.campaignList, "campaignid", r.id)
					if (o)
						Client.stackPushPage("PageTeacherCampaign.qml", {
												 group: control.group,
												 mapHandler: control.mapHandler,
												 campaign: o
											 })
				})
			})
			.fail(control, JS.failMessage(qsTr("Kihívás létrehozása sikertelen")))
		}
	}
}
