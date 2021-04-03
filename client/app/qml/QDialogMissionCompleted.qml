import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.3
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QDialogPanel {
	id: dialogPanel

	icon: CosStyle.iconTrophy
	titleColor: CosStyle.colorOK

	maximumWidth: 800
	maximumHeight: 600

	required property var gameData
	property bool demoMode: false

	Flickable {
		width: parent.width-20
		height: Math.min(parent.height, contentHeight)
		anchors.centerIn: parent

		contentWidth: col.width
		contentHeight: col.height

		clip: true

		flickableDirection: Flickable.VerticalFlick
		boundsBehavior: Flickable.StopAtBounds

		ScrollIndicator.vertical: ScrollIndicator { }

		Column {
			id: col
			width: parent.width

			property int streakMode: if (gameData.currentStreak > gameData.lastStreak) {
										 if (gameData.currentStreak > gameData.maxStreak)
											 2
										 else
											 1
									 } else {
										 0
									 }

			Row {
				id: rw0
				bottomPadding: 50
				anchors.horizontalCenter: parent.horizontalCenter

				spacing: 15

				QTrophyImage {
					height: CosStyle.pixelSize*3.5
					width: height

					visible: !demoMode

					anchors.verticalCenter: parent.verticalCenter

					isDeathmatch: gameData.deathmatch
					level: gameData.level
				}

				QLabel {
					anchors.verticalCenter: parent.verticalCenter
					text: qsTr("Mission completed")
					font.family: "HVD Peace"
					font.pixelSize: CosStyle.pixelSize*1.75
					color: CosStyle.colorOKLighter
				}
			}

			Row {
				id: rw1
				topPadding: 5
				bottomPadding: 5

				anchors.horizontalCenter: parent.horizontalCenter

				spacing: 10


				QLabel {
					anchors.verticalCenter: parent.verticalCenter
					id: xpLabel
					text: Math.floor(xp)+" XP"
					font.pixelSize: CosStyle.pixelSize*1.5
					font.weight: Font.DemiBold
					color: CosStyle.colorPrimaryLighter
					topPadding: 10
					bottomPadding: 10

					property real xp: 0

					Behavior on xp {
						NumberAnimation { duration: 750; easing.type: Easing.OutQuad }
					}
				}

				QMedalImage {
					visible: !demoMode && (gameData.solved === 1 || (gameData.deathmatch && gameData.deathmatchSolved === 1))
					anchors.verticalCenter: parent.verticalCenter
					width: height
					height: xpLabel.height
					level: gameData.level
					isDeathmatch: gameData.deathmatch
					image: gameData.medalImage
				}
			}

			QLabel {
				topPadding: 5
				bottomPadding: 5

				visible: !demoMode

				anchors.horizontalCenter: parent.horizontalCenter
				text: gameData.deathmatch ? qsTr("Sikeresen teljesítve: %1x").arg(gameData.deathmatchSolved) : qsTr("Sikeresen teljesítve: %1x").arg(gameData.solved)
				color: CosStyle.colorOKLighter
				font.pixelSize: CosStyle.pixelSize*0.9
				font.weight: Font.Medium
			}

			Row {
				visible: !demoMode

				topPadding: 70
				bottomPadding: 5

				spacing: 20

				anchors.horizontalCenter: parent.horizontalCenter

				QFontImage {
					id: streakIcon
					anchors.verticalCenter: parent.verticalCenter
					size: CosStyle.pixelSize*1.5
					icon: if (col.streakMode == 2)
							  CosStyle.iconStreakMax
						  else if (col.streakMode == 1)
							  CosStyle.iconStreakPlus
						  else
							  CosStyle.iconStreak
					color: if (col.streakMode == 2)
							   CosStyle.colorOKLighter
						   else if (col.streakMode == 1)
							   CosStyle.colorWarningLighter
						   else
							   CosStyle.colorAccent
				}

				QLabel {
					anchors.verticalCenter: parent.verticalCenter
					color: streakIcon.color
					font.weight: Font.Medium

					font.pixelSize: CosStyle.pixelSize*1.2

					text: qsTr("Streak: %1").arg(gameData.currentStreak)
				}

				QLabel {
					anchors.verticalCenter: parent.verticalCenter

					visible: gameData.streakXP

					color: CosStyle.colorPrimaryLighter
					font.pixelSize: CosStyle.pixelSize*1.5
					font.weight: Font.DemiBold

					text: qsTr("+%1 XP").arg(gameData.streakXP)
				}
			}

			ProgressBar {
				id: barStreak

				visible: !demoMode

				anchors.horizontalCenter: parent.horizontalCenter
				width: rw0.width*0.75
				from: 0
				to: Math.max(2, gameData.currentStreak+1, gameData.maxStreak)
				value: 0

				Material.accent: streakIcon.color

				Behavior on value {
					NumberAnimation { duration: 750; easing.type: Easing.OutQuad }
				}
			}

		}
	}


	buttons: QButton {
		id: buttonOk
		anchors.horizontalCenter: parent.horizontalCenter
		text: qsTr("OK")
		icon.source: CosStyle.iconOK
		themeColors: CosStyle.buttonThemeGreen

		onClicked: dialogPanel.dlgClose()
	}


	function populated() {
		buttonOk.forceActiveFocus()

		xpLabel.xp = gameData.xp

		if (!demoMode)
			barStreak.value = gameData.currentStreak
	}
}


