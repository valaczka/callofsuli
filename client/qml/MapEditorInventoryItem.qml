import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

QIconLoaderItemDelegate {
	id: root

	property MapEditorInventory inventory: null
	readonly property MapEditor editor: inventory && inventory.map ? inventory.map.mapEditor : null
	readonly property var _info: editor ? editor.inventoryInfo(inventory) : {}

	signal menuRequest(Item button)

	text: _info.name
	secondaryText: inventory && inventory.block > 0 ? qsTr("%1. csatatéren").arg(inventory.block) : ""

	selectableObject: inventory

	leftSourceComponent: Qaterial.Icon
	{
		size: Qaterial.Style.delegate.iconWidth

		icon: _view && _view.selectEnabled ?
				  (selectableObject && selectableObject.selected ? Qaterial.Icons.checkCircle : Qaterial.Icons.checkBold) :
				  (_info.icon !== undefined ? _info.icon : "")

		color: (_view && _view.selectEnabled) ? Qaterial.Style.accentColor : "transparent"
	}

	rightSourceComponent: Row {
		spacing: 3

		visible: _view && !_view.selectEnabled

		Qaterial.RoundButton {
			icon.source: Qaterial.Icons.minus
			icon.color: Qaterial.Colors.blue700
			anchors.verticalCenter: parent.verticalCenter
			enabled: inventory && inventory.count > 1
			onClicked: editor.missionLevelInventoryModify(missionLevel, inventory, function() {
				inventory.count--
			})
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
			onClicked: editor.missionLevelInventoryModify(missionLevel, inventory, function() {
				inventory.count++
			})
		}

		Qaterial.RoundButton {
			id: _btn
			icon.source: Qaterial.Icons.dotsVertical
			icon.color: Qaterial.Style.iconColor()
			anchors.verticalCenter: parent.verticalCenter
			onClicked: {
				if (_view)
					_view.currentIndex = root._index
				menuRequest(_btn)
			}

		}
	}
}
