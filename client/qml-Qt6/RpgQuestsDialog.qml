import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

Qaterial.ListDialog
{
	id: _dialog

	property RpgGameImpl game: null

	property string iconSource: ""
	property string text: ""
	property color iconColor: Qaterial.Style.accentColor
	property color textColor: Qaterial.Style.primaryTextColor()
	property int iconSize: 32 * Qaterial.Style.pixelSizeRatio
	property bool showFailed: false

	title: qsTr("Quests")

	dialogImplicitWidth: 450 * Qaterial.Style.pixelSizeRatio
	autoFocusButtons: true

	model: game ? game.quests : null

	delegate: _cmp

	standardButtons: DialogButtonBox.Ok

	listViewHeader: _dialog.iconSource != "" || _dialog.text != "" ? _cmpHeader : null

	Component {
		id: _cmpHeader

		Item {
			id: _headerItem

			width: ListView.view.width
			height: Math.max(_icon.implicitHeight, _text.implicitHeight, 70 * Qaterial.Style.pixelSizeRatio)

			Row {
				anchors.centerIn: parent
				spacing: Qaterial.Style.card.horizontalPadding

				Qaterial.RoundColorIcon
				{
					id: _icon

					anchors.verticalCenter: parent.verticalCenter
					highlighted: false
					fill: false
					visible: source != ""

					source: _dialog.iconSource
					iconSize: _dialog.iconSize
					color: _dialog.iconColor

				}

				Qaterial.LabelHeadline6
				{
					id: _text

					text: _dialog.text
					width: Math.min(implicitWidth,
									_headerItem.width - 2 * Qaterial.Style.card.horizontalPadding
									- (_icon.visible ? _icon.width + parent.spacing : 0)
									)
					color: _dialog.textColor
					wrapMode: Text.Wrap
					elide: Text.ElideRight
					maximumLineCount: 3

					anchors.verticalCenter: parent.verticalCenter
				}
			}
		}
	}

	Component {
		id: _cmp

		Qaterial.LoaderItemDelegate
		{
			id: _delegate

			property RpgQuest quest: modelData

			text: switch (quest.type) {
				  case RpgQuest.EnemyDefault: return qsTr("%1 killed enemies").arg(quest.amount)
				  case RpgQuest.WinnerDefault: return qsTr("%1 winner streak").arg(quest.amount)
				  case RpgQuest.SuddenDeath: return qsTr("Sudden death")
				  default: "--- invalid ---"
				  }

			secondaryText: switch (quest.type) {
						   case RpgQuest.SuddenDeath: return qsTr("You can't die")
						   default: ""
						   }

			width: ListView.view.width

			highlighted: quest.success > 0

			leftSourceComponent: Qaterial.RoundColorIcon
			{
				visible: source !== ""
				iconSize: Qaterial.Style.delegate.iconWidth

				fill: true
				width: roundIcon ? roundSize : iconSize
				height: roundIcon ? roundSize : iconSize
				color: _delegate.quest.success > 0 ? Qaterial.Style.accentColor : Qaterial.Colors.purple500

				source: switch (_delegate.quest.type){
						case RpgQuest.EnemyDefault:		return Qaterial.Icons.targetAccount
						case RpgQuest.WinnerDefault:	return Qaterial.Icons.checkBoxMultipleOutline
						case RpgQuest.SuddenDeath:		return Qaterial.Icons.skullScanOutline
						default: return ""
						}

			}

			rightSourceComponent: Row {
				spacing: 0

				Qaterial.Icon {
					visible: (_delegate.quest.success === 0 && showFailed) || (_delegate.quest.success < 0)
					icon: Qaterial.Icons.close
					color: Qaterial.Colors.red500
					size: Qaterial.Style.mediumIcon
					anchors.verticalCenter: parent.verticalCenter
				}

				Qaterial.Icon {
					visible: _delegate.quest.success > 0
					icon: Qaterial.Icons.checkCircle
					color: Qaterial.Colors.green500
					size: Qaterial.Style.mediumIcon
					anchors.verticalCenter: parent.verticalCenter
				}

				Qaterial.LabelSubtitle1 {
					visible: _delegate.quest.success > 1
					leftPadding: 3 * Qaterial.Style.pixelSizeRatio
					anchors.verticalCenter: parent.verticalCenter
					text: "×"+_delegate.quest.success
					color: Qaterial.Colors.green500
					/*font.family: Qaterial.Style.textTheme.subtitle1.family
					font.pixelSize: Qaterial.Style.textTheme.subtitle1.pixelSize
					font.weight: Font.Bold*/
				}

				Qaterial.Label {
					leftPadding: 15 * Qaterial.Style.pixelSizeRatio
					rightPadding: 5 * Qaterial.Style.pixelSizeRatio

					anchors.verticalCenter: parent.verticalCenter

					font.family: Qaterial.Style.textTheme.headline5.family
					font.pixelSize: Qaterial.Style.textTheme.headline5.pixelSize
					font.weight: Font.Medium

					color: Qaterial.Colors.yellow500

					text: Number(_delegate.quest.currency).toLocaleString()
				}

				Image {
					source: "qrc:/rpg/coin/coin.png"
					fillMode: Image.PreserveAspectFit
					height: 25 * Qaterial.Style.pixelSizeRatio
					width: 25 * Qaterial.Style.pixelSizeRatio

					anchors.verticalCenter: parent.verticalCenter
				}
			}
		}
	}
}
