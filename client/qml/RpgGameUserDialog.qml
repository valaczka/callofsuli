import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.ModalDialog
{
	id: _dialog

	required property ActionRpgMultiplayerGame game

	horizontalPadding: 0

	dialogImplicitWidth: 600

	autoFocusButtons: true

	property string iconSource: ""
	property string text: ""
	property color iconColor: Qaterial.Style.accentColor
	property color textColor: Qaterial.Style.primaryTextColor()
	property int iconSize: 32 * Qaterial.Style.pixelSizeRatio


	contentItem: GridLayout
	{
		id: _grid

		columns: 2

		RpgQuestHeader {
			id: _cmpHeader

			visible: iconSource != "" || text != ""

			iconSource: _dialog.iconSource
			text: _dialog.text
			iconColor: _dialog.iconColor
			textColor: _dialog.textColor
			iconSize: _dialog.iconSize

			Layout.fillWidth: true
			Layout.columnSpan: _grid.columns
		}

		Repeater {
			model: game ? game.playersModel : null

			delegate: Qaterial.ItemDelegate {
				Layout.fillHeight: true
				Layout.fillWidth: true

				text: username + " " + nickname + " - " + character
				secondaryText: xp + " XP, " + cur + " CURR " + finished
							   + (player ? " " + player.hp + " HP" : "")
							   + (player ? " " + player.collection + "/" + player.collectionRq : "")

				icon.source: Qaterial.Icons.account

				onClicked: {

				}
			}
		}

		Repeater {
			model : game && game.rpgGame ? game.rpgGame.quests : null

			delegate: RpgQuestDelegate {
				showFailed: game && game.config.gameState === RpgConfig.StateFinished

				Layout.fillWidth: true
				Layout.columnSpan: _grid.columns
			}
		}
	}

	standardButtons: DialogButtonBox.Ok

	Component.onCompleted: {
		open()
	}


	onOpened: {

	}

	onAccepted: {

	}



}

