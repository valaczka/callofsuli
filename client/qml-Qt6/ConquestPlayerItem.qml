import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


Rectangle {
	id: root

	implicitHeight: _col.implicitHeight
	implicitWidth: 100

	property ConquestGame game: null
	property int playerId: -1
	property string username: ""
	property string fullNickName: ""
	property string character: "default"
	property int xp: 0

	property Item targetFighter1: null
	property Item targetFighter2: null

	color: game && playerId != -1 && game.currentTurn.player === playerId ?
			   Client.Utils.colorSetAlpha(_playerColor, 0.85) :
			   Client.Utils.colorSetAlpha(Qaterial.Colors.black, 0.85)

	readonly property color _playerColor: game ? Qt.darker(game.getPlayerColor(playerId), 1.5) : Qaterial.Colors.black

	radius: 5

	Behavior on xp {
		NumberAnimation { duration: 650; easing.type: Easing.OutQuad }
	}

	Behavior on color {
		ColorAnimation { duration: 450 }
	}

	Item {
		id: _placeholder
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		width: height

		Image {
			id: _playerItem
			fillMode: Image.PreserveAspectFit
			width: _placeholder.width*0.9
			height: _placeholder.height*0.9
			/*x: (parent.width-width)/2
			y: (parent.height-height)/2*/
			anchors.centerIn: parent

			source: "qrc:/character/%1/thumbnail.png".arg(character)
		}
	}

	Column {
		id: _col
		anchors.left: _placeholder.right
		anchors.right: parent.right
		anchors.top: parent.top
		anchors.bottom: parent.bottom

		Qaterial.LabelSubtitle2 {
			id: _labelName

			width: parent.width
			elide: width < implicitWidth ? Text.ElideRight : Text.ElideNone
			text: fullNickName
			rightPadding: 5 * Qaterial.Style.pixelSizeRatio
			color: Qaterial.Style.primaryTextColor()
		}

		Row {
			anchors.left: parent.left

			Qaterial.LabelCaption {
				id: _labelXP
				anchors.verticalCenter: parent.verticalCenter
				color: Qaterial.Style.primaryTextColor()

				//width: parent.width

				text: "%1 XP".arg(xp)
			}
		}
	}

	readonly property bool _isFighter1: game && game.fighter1.playerId === playerId && playerId != -1 &&
										game.currentStage == ConquestTurn.StageBattle &&
										targetFighter1

	readonly property bool _isFighter2: game && game.fighter2.playerId === playerId && playerId != -1 &&
										game.currentStage == ConquestTurn.StageBattle &&
										targetFighter2

	/*states: [
		State {
			name: "nofight"
			when: !_isFighter1 && !_isFighter2
			ParentChange {
				target: _playerItem
				parent: _placeholder
				x: (_placeholder.width- _playerItem.width)/2
				y: (_placeholder.height- _playerItem.height)/2
			}
		},
		State {
			name: "reparented1"
			when: _isFighter1
			ParentChange {
				target: _playerItem
				parent: targetFighter1
				x: (targetFighter1.width- _playerItem.width)/2
				y: (targetFighter1.height- _playerItem.height)/2
			}
		},
		State {
			name: "reparented2"
			when: _isFighter2
			ParentChange {
				target: _playerItem
				parent: targetFighter2
				x: (targetFighter2.width- _playerItem.width)/2
				y: (targetFighter2.height- _playerItem.height)/2
			}
		}
	]

	transitions: Transition {
		ParentAnimation {
			NumberAnimation {
				properties: "x,y"
				duration: 350
			}
		}
	}*/
}
