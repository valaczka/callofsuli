import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import "JScript.js" as JS

QItemGradient {
	id: root

	property StudentMapHandler studentMapHandler: null
	property User user: Client.server ? Client.server.user : null
	property CampaignList campaignList: Client.cache("studentCampaignList")

	property bool _firstRun: true

	QScrollable {
		anchors.fill: parent
		spacing: 15
		contentCentered: true

		refreshEnabled: true

		onRefreshRequest: reload()

		Qaterial.LabelHeadline3 {
			anchors.horizontalCenter: parent.horizontalCenter
			width: Math.min(parent.width-100, Qaterial.Style.maxContainerSize)
			topPadding: root.paddingTop
			horizontalAlignment: Qt.AlignHCenter
			text: user ? user.fullNickName : ""
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
				}
			}
		}

		Column {
			id: _col
			width: Math.min(parent.width-40, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter
			spacing: 0
			topPadding: 40

			QAnimatedProgressBar {
				id: _progressXp
				width: Math.min(parent.width, 450)
				anchors.horizontalCenter: parent.horizontalCenter
				from: 0
				value: 0
				to: 1
				textFormat: "%1 XP"
				icon: Qaterial.Icons.graphOutline
			}

			Qaterial.LabelHint1 {
				anchors.right: _progressXp.right
				visible: text != ""

				readonly property int rankId: user ? user.rank.id : -1

				onRankIdChanged: {
					if (!Client.server || rankId < 0) {
						text = ""
						_progressXp.rightText = ""
						return
					}

					var r = Client.server.nextRank(Client.server.rank(rankId))

					if (r.id < 0) {
						text = ""
						_progressXp.rightText = ""
						return
					}

					_progressXp.rightText = "%1 XP".arg(r.xp.toLocaleString())
					_progressXp.from = Math.max(user.rank.xp, 0)
					_progressXp.to = r.xp
					text = qsTr("%1 (lvl %2)").arg(r.name).arg(r.sublevel)
				}
			}
		}


		QDashboardGrid {
			id: _grid
			anchors.horizontalCenter: parent.horizontalCenter

			readonly property bool showPlaceholders: _campaignList.count === 0 && _firstRun

			visible: _campaignList.count || showPlaceholders
			contentItems: showPlaceholders ? 3 : _campaignList.count

			Repeater {
				model: _grid.showPlaceholders ? 3 : _campaignList

				delegate: _grid.showPlaceholders ? _cmpPlaceholder : _cmpButton
			}

			Component {
				id: _cmpButton

				QDashboardButton {
					id: _btn
					property Campaign campaign: model && model.qtObject ? model.qtObject : null
					text: campaign ? campaign.readableName : ""

					icon.source: Qaterial.Icons.playBox

					onClicked: Client.stackPushPage("PageStudentCampaign.qml", {
														user: root.user,
														campaign: _btn.campaign,
														studentMapHandler: root.studentMapHandler
													})
				}
			}

			Component {
				id: _cmpPlaceholder

				QPlaceholderItem {
					widthRatio: 1.0
					heightRatio: 1.0
					width: _grid.buttonSize
					height: _grid.buttonSize
					rectangleRadius: 5
				}
			}

		}

	}

	SortFilterProxyModel {
		id: _campaignList
		sourceModel: campaignList

		filters: ValueFilter {
			roleName: "finished"
			value: false
		}

		sorters: StringSorter {
			roleName: "readableName"
			sortOrder: Qt.AscendingOrder
		}
	}


	Connections {
		target: user

		function onXpChanged() {
			if (root.StackView.status == StackView.Active)
				_progressXp.value = user.xp
		}
	}

	StackView.onActivated: {
		reload()
		if (!user)
			return
		_progressXp.value = user.xp
	}

	StackView.onDeactivated: {
		if (!user)
			return
		_progressXp.value = 0
	}

	function reload() {
		Client.reloadUser()
		Client.reloadCache("studentCampaignList", function() {
			_firstRun = false
		})
	}
}


