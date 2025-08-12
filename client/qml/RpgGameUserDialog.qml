import QtQuick
import QtQuick.Controls
import SortFilterProxyModel
import Qt5Compat.GraphicalEffects
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


Qaterial.ListDialog
{
	id: _dialog

	required property ActionRpgMultiplayerGame game

	property string iconSource: ""
	property string text: ""
	property color iconColor: Qaterial.Style.accentColor
	property color textColor: Qaterial.Style.primaryTextColor()
	property int iconSize: 32 * Qaterial.Style.pixelSizeRatio

	title: qsTr("Players & Quests")

	dialogImplicitWidth: 600 * Qaterial.Style.pixelSizeRatio
	autoFocusButtons: true

	standardButtons: DialogButtonBox.Ok

	listViewHeader: Column
	{
		width: ListView.view.width

		RpgQuestHeader {
			id: _cmpHeader

			width: parent.width

			visible: iconSource != "" || text != ""

			iconSource: _dialog.iconSource
			text: _dialog.text
			iconColor: _dialog.iconColor
			textColor: _dialog.textColor
			iconSize: _dialog.iconSize
		}

		ListView {
			width: parent.width
			height: Math.min(_dialog.height*0.3, 120)

			boundsBehavior: Flickable.StopAtBounds
			snapMode: ListView.SnapToItem
			clip: true
			orientation: ListView.Horizontal

			spacing: 5

			model: SortFilterProxyModel {
				sourceModel: game ? game.playersModel : null

				sorters: [
					FilterSorter {
						ValueFilter {
							roleName: "playerId"
							value: game ? game.playerId : -1
						}
					}

				]
			}

			header: Item {
				width: ListView.view.spacing
				height: ListView.view.height
			}

			footer: Item {
				width: ListView.view.spacing
				height: ListView.view.height
			}

			delegate: RpgSelectCard {
				height: Math.min(_dialog.height*0.3, 120)
				width: height
				text: nickname
				image: game ? game.getCharacterImage(character) : ""
				selected: finished
				scale: 1.0

				Qaterial.LabelHeadline6 {
					id: _label
					anchors.right: parent.right
					anchors.top: parent.top
					anchors.margins: 10

					text: (player ? " " + player.collection + " / " + player.collectionRq : "")

					color: Qaterial.Colors.green400
				}

				Glow {
					anchors.fill: _label
					source: _label
					color: "black"
					radius: 1
					spread: 0.9
					samples: 5
				}
			}
		}
	}

	model: game && game.rpgGame ? game.rpgGame.quests : null

	delegate: Component {
		RpgQuestDelegate {
			showFailed: game && game.config.gameState === RpgConfig.StateFinished

			width: ListView.view.width
		}
	}

}

