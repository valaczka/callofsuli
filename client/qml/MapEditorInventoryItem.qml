import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

QIconLoaderItemDelegate {
	id: root

	property MapEditorInventory inventory: null
	readonly property MapEditor editor: inventory && inventory.map ? inventory.map.mapEditor : null
	readonly property var _info: editor ? editor.inventoryInfo(inventory) : {}

	text: _info.name
	secondaryText: inventory && inventory.block > 0 ? qsTr("%1. csatatér").arg(inventory.block) : ""

	selectableObject: inventory

	leftSourceComponent: Qaterial.Icon
	{
		size: Qaterial.Style.delegate.iconWidth

		icon: _view && _view.selectEnabled ?
						 selectableObject && selectableObject.selected ?
							 Qaterial.Icons.checkCircle : Qaterial.Icons.checkBold : _info.icon

		color: (_view && _view.selectEnabled) ? Qaterial.Style.accentColor : "transparent"
	}

	rightSourceComponent: Row {
		spacing: 3

		Qaterial.RoundButton {
			icon.source: Qaterial.Icons.minus
			icon.color: Qaterial.Colors.blue700
			anchors.verticalCenter: parent.verticalCenter
			enabled: inventory && inventory.count > 1
			//onClicked: editor.mo
		}

		QBanner {
			num: inventory ? inventory.count : 0
			visible: num > 0
			color: Qaterial.Colors.blue600
			anchors.verticalCenter: parent.verticalCenter
		}

		Qaterial.RoundButton {
			icon.source: Qaterial.Icons.plus
			icon.color: Qaterial.Colors.blue700
			anchors.verticalCenter: parent.verticalCenter
			enabled: inventory && inventory.count < 100
			//onClicked: editor.mo
		}

		Qaterial.RoundButton {
			icon.source: Qaterial.Icons.dotsVertical
			icon.color: Qaterial.Style.iconColor()
			anchors.verticalCenter: parent.verticalCenter
		}

	}
}
