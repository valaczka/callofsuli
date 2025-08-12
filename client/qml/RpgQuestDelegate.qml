import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

Qaterial.LoaderItemDelegate
{
    id: _delegate

    property bool showFailed: false

    property RpgQuest quest: modelData

    text: switch (quest.type) {
          case RpgQuest.EnemyDefault: return qsTr("%1 killed enemies").arg(quest.amount)
          case RpgQuest.WinnerDefault: return qsTr("%1 winner streak").arg(quest.amount)
          case RpgQuest.SuddenDeath: return qsTr("Sudden death")
          case RpgQuest.NoKill: return qsTr("No killing")
          case RpgQuest.Collect: return qsTr("Collect %1 items").arg(quest.amount)
          default: "--- invalid ---"
          }

    secondaryText: switch (quest.type) {
                   case RpgQuest.SuddenDeath: return qsTr("You must not die")
                   case RpgQuest.NoKill: return qsTr("You must not kill anyone")
                   default: ""
                   }

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
                case RpgQuest.NoKill:       	return Qaterial.Icons.skullCrossbones
                case RpgQuest.Collect:       	return Qaterial.Icons.mapMarkerMultipleOutline
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
