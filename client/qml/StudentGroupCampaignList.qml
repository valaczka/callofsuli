import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import SortFilterProxyModel


QItemGradient {
	id: control

	property StudentMapHandler mapHandler: null
	property StudentGroupList groupList: null
	property StudentGroup group: null

	signal changeGroup(StudentGroup group)

	title: group ? group.name+qsTr(" | kihívások") : ""

	appBar.rightComponent: StudentGroupButton {
		anchors.verticalCenter: parent.verticalCenter
		groupList: control.groupList
		group: control.group
		onChangeGroup: group => control.changeGroup(group)
	}

	QScrollable {
		anchors.fill: parent
		topPadding: Math.max(Client.safeMarginTop, control.paddingTop + 5)
		leftPadding: 0
		bottomPadding: 0
		rightPadding: 0

		refreshEnabled: true
		onRefreshRequest: Client.reloadCache("studentCampaignList")

		QListView {
			id: view

			currentIndex: -1
			autoSelectChange: false

			height: contentHeight
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			boundsBehavior: Flickable.StopAtBounds

			model: SortFilterProxyModel {
				sourceModel: Client.cache("studentCampaignList")

				filters: [
					ValueFilter {
						roleName: "groupid"
						value: group ? group.groupid : -1
					}

				]

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


			delegate: Qaterial.LoaderItemDelegate {
				id: _delegate

				width: view.width

				property Campaign campaign: model.qtObject

				highlighted: ListView.isCurrentItem

				readonly property string iconSource: {
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

				readonly property color iconColor: {
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

				text: campaign ? campaign.readableName : ""

				secondaryText: {
					if (!campaign)
						return ""

					if (campaign.startTime.getTime()) {
						return campaign.startTime.toLocaleString(Qt.locale(), "yyyy. MMM d. HH:mm – ")
								+ (campaign.endTime.getTime() ? campaign.endTime.toLocaleString(Qt.locale(), "yyyy. MMM d. HH:mm") : "")
					}

					return ""
				}

				textColor: iconColor
				secondaryTextColor: !campaign || campaign.state == Campaign.Finished ?
										Qaterial.Style.disabledTextColor() : Qaterial.Style.colorTheme.secondaryText

				leftSourceComponent: Qaterial.RoundColorIcon
				{
					source: _delegate.iconSource
					color: _delegate.iconColor
					iconSize: Qaterial.Style.delegate.iconWidth

					fill: true
					width: roundIcon ? roundSize : iconSize
					height: roundIcon ? roundSize : iconSize
				}

				rightSourceComponent: Qaterial.LabelHeadline5 {
					visible: text != ""
					text: {
						if (!_delegate.campaign)
							return ""

						let l = []

						if (_delegate.campaign.maxPts > 0)
							l.push(qsTr("%1 pt").arg(Number(Math.round(_delegate.campaign.maxPts * _delegate.campaign.progress).toLocaleString())))

						if (_delegate.campaign.resultXP > 0)
							l.push(qsTr("%1 XP").arg(Number(_delegate.campaign.resultXP).toLocaleString()))

						if (_delegate.campaign.resultGrade)
							l.push(_delegate.campaign.resultGrade.shortname)

						return l.join(" / ")
					}
					color: Qaterial.Style.accentColor
				}

				onClicked: Client.stackPushPage("PageStudentCampaign.qml", {
													user: Client.server ? Client.server.user : null,
													campaign: campaign,
													studentMapHandler: control.mapHandler,
													withResult: true,
													title: group ? group.name : ""
												})
			}
		}

	}


}
