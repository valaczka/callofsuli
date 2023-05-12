import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

QIconLoaderItemDelegate {
	id: root

	property MapEditorObjective objective: null
	readonly property MapEditor editor: objective && objective.map ? objective.map.mapEditor : null
	readonly property var _info: editor ? editor.objectiveInfo(objective) : {}

	iconSource: _info.icon
	text: _info.title
	secondaryText: _info.details

	selectableObject: objective

	rightSourceComponent: Row {
		spacing: 3

		Qaterial.RoundColorIcon
		{
			anchors.verticalCenter: parent.verticalCenter
			iconSize: Qaterial.Style.delegate.iconWidth

			fill: true
			width: roundIcon ? roundSize : iconSize
			height: roundIcon ? roundSize : iconSize

			visible: source != ""
			source: editor && objective && objective.storage ? editor.storageInfo(objective.storage).icon : ""

			color: Qaterial.Colors.green400
		}

		QBanner {
			num: objective ? objective.storageCount : 0
			visible: num > 0
			color: Qaterial.Colors.cyan900
			anchors.verticalCenter: parent.verticalCenter
		}

		Qaterial.RoundButton {
			icon.source: Qaterial.Icons.dotsVertical
			icon.color: Qaterial.Style.iconColor()
			anchors.verticalCenter: parent.verticalCenter
		}

	}
}