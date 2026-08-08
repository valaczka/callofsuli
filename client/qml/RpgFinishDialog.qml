import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


RpgDialog {
	id: panel

	required property RpgGame game
	required property var result
	property bool _active: false

	anchors.fill: parent

	borderColor: result.success ? Qaterial.Colors.green400 : Qaterial.Colors.red400

	dialogImplicitWidth: 650
	dialogImplicitHeight: _col.implicitHeight+85

	onActivated: _active = true

	Column {
		id: _col
		visible: !result.isTutorial

		anchors.centerIn: parent
		width: parent.width
		spacing: 5

		Label {
			color: panel.borderColor
			font.family: "HVD Peace"
			font.pixelSize: Qaterial.Style.textTheme.headline3.pixelSize
			text: result.success ? qsTr("Mission completed") : qsTr("Mission failed")
			width: parent.width*0.8
			horizontalAlignment: Text.AlignHCenter
			wrapMode: Text.Wrap
			anchors.horizontalCenter: parent.horizontalCenter
			lineHeight: 0.85
		}

		Qaterial.LabelHeadline6 {
			text: qsTr("Quests")
			anchors.horizontalCenter: parent.horizontalCenter
			topPadding: 20
		}

		Row {
			id: _rowPts

			visible: result.ptsRq > 0

			readonly property bool _success: game.ptsTeam >= result.ptsRq

			anchors.horizontalCenter: parent.horizontalCenter

			spacing: 5

			Qaterial.Icon {
				color: _rowPts._success ? Qaterial.Colors.green500 : Qaterial.Colors.red500
				icon: "qrc:/rpg/coin/coins.png"
				anchors.verticalCenter: parent.verticalCenter
			}

			Qaterial.LabelHint1 {
				text: game.ptsTeam
				anchors.verticalCenter: parent.verticalCenter
			}

			ProgressBar {
				anchors.verticalCenter: parent.verticalCenter

				width: 100

				from: 0
				to: result.ptsRq
				value: _active ? game.ptsTeam : 0

				Material.accent: color

				property color color: _rowPts._success ? Qaterial.Colors.green500 : Qaterial.Colors.red500

				Behavior on value {
					NumberAnimation { duration: 750; easing.type: Easing.InOutQuad }
				}
			}

			Qaterial.Icon {
				color: _rowPts._success ? Qaterial.Colors.green500 : Qaterial.Colors.red500
				icon: _rowPts._success ? Qaterial.Icons.checkCircle : Qaterial.Icons.close
				anchors.verticalCenter: parent.verticalCenter
			}
		}

		Row {
			id: _rowQuestion

			readonly property bool _success: result.question >= result.questionRq

			anchors.horizontalCenter: parent.horizontalCenter

			spacing: 5

			Qaterial.Icon {
				color: _rowQuestion._success ? Qaterial.Colors.green500 : Qaterial.Colors.red500
				icon: Qaterial.Icons.abacus
				anchors.verticalCenter: parent.verticalCenter
			}

			Qaterial.LabelHint1 {
				text: result.question
				anchors.verticalCenter: parent.verticalCenter
			}

			ProgressBar {
				anchors.verticalCenter: parent.verticalCenter

				width: 100

				from: 0
				to: result.questionRq
				value: _active ? result.question : 0

				Material.accent: color

				property color color: _rowQuestion._success ? Qaterial.Colors.green500 : Qaterial.Colors.red500

				Behavior on value {
					NumberAnimation { duration: 750; easing.type: Easing.InOutQuad }
				}
			}


			Qaterial.Icon {
				color: _rowQuestion._success ? Qaterial.Colors.green500 : Qaterial.Colors.red500
				icon: _rowQuestion._success ? Qaterial.Icons.checkCircle : Qaterial.Icons.close
				anchors.verticalCenter: parent.verticalCenter
			}
		}

		Row {
			id: _rowStreak

			readonly property bool _success: result.streak >= result.streakRq

			anchors.horizontalCenter: parent.horizontalCenter

			spacing: 5

			Qaterial.Icon {
				color: _rowStreak._success ? Qaterial.Colors.green500 : Qaterial.Colors.red500
				icon: Qaterial.Icons.abacus
				anchors.verticalCenter: parent.verticalCenter
			}

			Qaterial.LabelHint1 {
				text: result.streak
				anchors.verticalCenter: parent.verticalCenter
			}

			ProgressBar {
				anchors.verticalCenter: parent.verticalCenter

				width: 100

				from: 0
				to: result.streakRq
				value: _active ? result.streak : 0

				Material.accent: color

				property color color: _rowStreak._success ? Qaterial.Colors.green500 : Qaterial.Colors.red500

				Behavior on value {
					NumberAnimation { duration: 750; easing.type: Easing.InOutQuad }
				}
			}


			Qaterial.Icon {
				color: _rowStreak._success ? Qaterial.Colors.green500 : Qaterial.Colors.red500
				icon: _rowStreak._success ? Qaterial.Icons.checkCircle : Qaterial.Icons.close
				anchors.verticalCenter: parent.verticalCenter
			}
		}


		Qaterial.LabelHeadline6 {
			readonly property real _heatFactor: result.success && game.gameMode == RpgGame.SinglePlayer ? (1+game.heat*0.5) : 0

			text: _heatFactor > 1 ? qsTr("Rewards (x%1)").arg(_heatFactor) : qsTr("Rewards")
			anchors.horizontalCenter: parent.horizontalCenter
			topPadding: 20
		}

		Qaterial.LabelHint1 {
			anchors.horizontalCenter: parent.horizontalCenter
			visible: result.success && game.heat > 0 && game.gameMode == RpgGame.SinglePlayer
			text: qsTr("Danger level: %1").arg(game.heat)
		}

		Row {
			anchors.horizontalCenter: parent.horizontalCenter

			spacing: 40


			Row {
				anchors.verticalCenter: parent.verticalCenter
				spacing: 5

				Qaterial.Icon {
					icon: "qrc:/rpg/coin/coins.png"
					color: "transparent"
					anchors.verticalCenter: parent.verticalCenter
				}

				Qaterial.LabelHeadline4 {
					property int pts: _active ? result.pts : 0
					anchors.verticalCenter: parent.verticalCenter
					text: pts
					color: Qaterial.Colors.amber400

					Behavior on pts {
						NumberAnimation { duration: 1250; easing.type: Easing.OutQuad }
					}
				}
			}

			Row {
				anchors.verticalCenter: parent.verticalCenter
				spacing: 5

				Qaterial.Icon {
					color: Qaterial.Colors.blue400
					icon: Qaterial.Icons.abacus
					anchors.verticalCenter: parent.verticalCenter
				}

				Qaterial.LabelHeadline4 {
					property int pts: _active ? result.tokenReal : 0
					anchors.verticalCenter: parent.verticalCenter
					text: pts
					color: Qaterial.Colors.blue400

					Behavior on pts {
						NumberAnimation { duration: 1250; easing.type: Easing.OutQuad }
					}
				}
			}


			Qaterial.LabelHeadline4 {
				property int pts: _active ? result.xpReal : 0

				Behavior on pts {
					NumberAnimation { duration: 1250; easing.type: Easing.OutQuad }
				}

				anchors.verticalCenter: parent.verticalCenter
				text: pts+" XP"
			}
		}
	}


	Label {
		visible: result.isTutorial !== undefined
		color: Qaterial.Colors.green500
		font.family: "HVD Peace"
		font.pixelSize: Qaterial.Style.textTheme.headline3.pixelSize
		text: qsTr("Tutorial completed")
		width: parent.width*0.8
		horizontalAlignment: Text.AlignHCenter
		wrapMode: Text.Wrap
		anchors.centerIn: parent
		lineHeight: 0.85
	}


	QButton {
		icon.source: Qaterial.Icons.checkBold
		text: qsTr("OK")
		anchors.right: parent.right
		anchors.bottom: parent.bottom
		onClicked: panel.closeRequest()
	}

}

