import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QSwipeContainer {
	id: panel

	title: qsTr("Eredmények")
	icon: CosStyle.iconXPgraph

	QLabel {
		visible: !flickable.visible
		anchors.centerIn: parent
		text: qsTr("Válassz felhasználót")
	}


	Flickable {
		id: flickable
		visible: false
		width: parent.width-20
		height: Math.min(parent.height, contentHeight)
		anchors.centerIn: parent

		contentWidth: col.width
		contentHeight: col.height

		clip: true

		flickableDirection: Flickable.VerticalFlick
		boundsBehavior: Flickable.StopAtBounds

		ScrollIndicator.vertical: ScrollIndicator { }

		readonly property real _contentWidth: Math.min(width, 600)

		Column {
			id: col
			width: flickable.width

			spacing: 10

			ProfileDetailsUser {
				id: user

				anchors.horizontalCenter: parent.horizontalCenter

				width: parent.width
			}

			ProfileDetailsProgressBar {
				id: barXP
				anchors.horizontalCenter: parent.horizontalCenter
				width: flickable._contentWidth

				icon: CosStyle.iconXPgraph
				color: CosStyle.colorPrimaryLighter
				labelFormat: "%1 XP"
			}

			ProfileDetailsProgressBar {
				id: barStreak
				anchors.horizontalCenter: parent.horizontalCenter
				width: flickable._contentWidth

				icon: CosStyle.iconStreak
				color: CosStyle.colorAccent
			}

			ProfileDetailsProgressBar {
				id: barStreakMax
				anchors.horizontalCenter: parent.horizontalCenter
				width: flickable._contentWidth

				icon: CosStyle.iconStreakMax
				color: CosStyle.colorAccentLighter
			}

			QLabel {
				id: labelTitle
				text: qsTr("Trófeák")

				color: CosStyle.colorPrimaryLight
				font.pixelSize: CosStyle.pixelSize*0.9
				font.weight: Font.DemiBold
				font.capitalization: Font.AllUppercase

				anchors.horizontalCenter: parent.horizontalCenter
				topPadding: 30

				opacity: 0.3
			}

			ProfileDetailsTrophies {
				id: trophies

				topPadding: 10
				bottomPadding: 25

				imageSize: CosStyle.pixelSize*2

				width: barStreakMax.barWidth
				anchors.horizontalCenter: parent.horizontalCenter
			}


			ProfileDetailsProgressBar {
				id: barT1
				anchors.horizontalCenter: parent.horizontalCenter
				width: flickable._contentWidth

				level: 1
				deathmatch: false
				color: "limegreen"
			}

			ProfileDetailsProgressBar {
				id: barD1
				anchors.horizontalCenter: parent.horizontalCenter
				width: flickable._contentWidth

				level: 1
				deathmatch: true
				color: "lime"
			}


			ProfileDetailsProgressBar {
				id: barT2
				anchors.horizontalCenter: parent.horizontalCenter
				width: flickable._contentWidth

				level: 2
				deathmatch: false
				color: "peru"
			}

			ProfileDetailsProgressBar {
				id: barD2
				anchors.horizontalCenter: parent.horizontalCenter
				width: flickable._contentWidth

				level: 2
				deathmatch: true
				color: "sandybrown"
			}


			ProfileDetailsProgressBar {
				id: barT3
				anchors.horizontalCenter: parent.horizontalCenter
				width: flickable._contentWidth

				level: 3
				deathmatch: false
				color: "goldenrod"
			}

			ProfileDetailsProgressBar {
				id: barD3
				anchors.horizontalCenter: parent.horizontalCenter
				width: flickable._contentWidth

				level: 3
				deathmatch: true
				color: "gold"
			}
		}

	}


	function loadUserScore(jsonData) {
		flickable.visible = true

		user.image = cosClient.rankImageSource(jsonData.rankid, -1, jsonData.rankimage)

		user.username = jsonData.nickname.length ? jsonData.nickname : jsonData.firstname+" "+jsonData.lastname
		user.rankname = jsonData.rankname+(jsonData.ranklevel > 0 ? qsTr(" (lvl %1)").arg(jsonData.ranklevel) : "")

		trophies.t1 = jsonData.t1
		trophies.t2 = jsonData.t2
		trophies.t3 = jsonData.t3
		trophies.d1 = jsonData.d1
		trophies.d2 = jsonData.d2
		trophies.d3 = jsonData.d3

		if (jsonData.t1+jsonData.t2+jsonData.t3+jsonData.d1+jsonData.d2+jsonData.d3)
			labelTitle.opacity = 1.0
		else
			labelTitle.opacity = 0.3

		barXP.to = jsonData.maxXP
		barXP.value = jsonData.xp

		barStreak.to = jsonData.maxStreak
		barStreak.value = jsonData.currentStreak

		barStreakMax.to = jsonData.maxStreak
		barStreakMax.value = jsonData.longestStreak

		barT1.to = jsonData.maxT1
		barT2.to = jsonData.maxT2
		barT3.to = jsonData.maxT3
		barD1.to = jsonData.maxD1
		barD2.to = jsonData.maxD2
		barD3.to = jsonData.maxD3

		barT1.value = jsonData.t1
		barT2.value = jsonData.t2
		barT3.value = jsonData.t3

		barD1.value = jsonData.d1
		barD2.value = jsonData.d2
		barD3.value = jsonData.d3
	}
}
