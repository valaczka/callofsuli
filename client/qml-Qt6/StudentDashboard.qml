import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import SortFilterProxyModel
import "JScript.js" as JS

QItemGradient {
	id: root

	property StudentMapHandler studentMapHandler: null
	property User user: Client.server ? Client.server.user : null
	property CampaignList campaignList: Client.cache("studentCampaignList")

	property bool _firstRun: true
	property bool _notifyDailyRate80: false
	property bool _notifyDailyRate100: false

	appBar.rightComponent: Qaterial.AppBarButton {
		icon.source: Qaterial.Icons.cogOutline
		onClicked: Client.stackPushPage("PageStudentSettings.qml")
	}

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
			topPadding: 20 * Qaterial.Style.pixelSizeRatio

			QAnimatedProgressBar {
				id: _progressXp
				width: Math.min(parent.width, 450)
				anchors.horizontalCenter: parent.horizontalCenter
				from: 0
				value: 0
				to: 1
				textFormat: "%1 XP"
				icon: Qaterial.Icons.chartTimelineVariantShimmer
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


		StudentStreak {
			id: _streakRow
			anchors.horizontalCenter: parent.horizontalCenter
			duration: _progressXp.duration
			value: 0
			user: root.user
			maxIconCount: Math.floor((_col.width-implicitLabelWidth)/iconSize)
		}


		StudentXpChart {
			id: _chart
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize, 768*Qaterial.Style.pixelSizeRatio*0.85)
			anchors.horizontalCenter: parent.horizontalCenter
			visible: false

			property bool _showPlaceholder: true
		}


		QAnimatedProgressBar {
			id: _progressLimit
			width: Math.min(parent.width-40, Qaterial.Style.maxContainerSize, 450)
			anchors.horizontalCenter: parent.horizontalCenter
			spacing: 0
			topPadding: 20 * Qaterial.Style.pixelSizeRatio
			color: Qaterial.Colors.amber500

			visible: user && user.dailyRate >= 0.8

			from: 0
			value: user ? user.dailyRate*100 : 0
			to: 100
			textFormat: qsTr("Napi játékidő: %1%")
			icon: Qaterial.Icons.timerAlert
		}


		QPlaceholderItem {
			anchors.horizontalCenter: parent.horizontalCenter
			widthRatio: 1.0
			heightRatio: 1.0
			width: _chart.width
			height: _chart.height
			rectangleRadius: 5
			visible: _chart._showPlaceholder
		}

		QButton {
			text: "DEMO"
			onClicked: {
				onClicked: Client.loadDemoMap()
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

					icon.source: Qaterial.Icons.trophyBroken

					onClicked: {
						let group = null

						if (campaign && campaign.groupid > -1)
							group = Client.findCacheObject("studentGroupList", campaign.groupid)

						Client.stackPushPage("PageStudentCampaign.qml", {
												 user: root.user,
												 campaign: _btn.campaign,
												 studentMapHandler: root.studentMapHandler,
												 title: group ? group.name : ""
											 })
					}
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

		function onStreakChanged() {
			if (root.StackView.status == StackView.Active)
				_streakRow.value = user.streak
		}

		function onDailyRateChanged() {
			if (user.dailyRate >= 1.0 && !_notifyDailyRate100) {
				Client.messageWarning(qsTr("Elérted a napi játékidőt, ma már nem tudsz több játékot indítani!"), qsTr("Játékidő"))
				_notifyDailyRate100 = true
			} else if (user.dailyRate >= 0.8 && !_notifyDailyRate80) {
				Client.messageInfo(qsTr("Elérted a napi játékidő 80%-át"), qsTr("Játékidő"))
				_notifyDailyRate80 = true
			}
		}
	}

	StackView.onActivated: {
		reload()
		if (!user)
			return
		_progressXp.value = user.xp
		_streakRow.value = user.streak
	}

	StackView.onDeactivated: {
		if (!user)
			return
		_progressXp.value = 0
		_streakRow.value = user.streak
	}

	function reload() {
		Client.reloadUser()
		Client.reloadCache("studentCampaignList", root, function() {
			_firstRun = false
		})
		Client.send(HttpConnection.ApiGeneral, "user/%1/log/xp".arg(user.username))
		.done(root, function(r){
			_chart.loadList(r.list)
			_chart._showPlaceholder = false
			if (r.list.length)
				_chart.visible = true
		})
		.fail(root, (err) => _chart._showPlaceholder = false)
	}
}


