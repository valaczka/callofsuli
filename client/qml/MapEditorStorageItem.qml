import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

QIconLoaderItemDelegate {
	id: root

	property MapEditorStorage storage: null
	readonly property MapEditor editor: storage && storage.map ? storage.map.mapEditor : null
	property var _info: editor ? editor.storageInfo(storage) : {}

	textColor: Qaterial.Colors.green400
	iconColor: textColor

	iconSource: _info.icon !== undefined ? _info.icon : ""
	text: _info.name !== undefined ? _info.name : ""
	secondaryText: _info.title !== undefined ? _info.title : ""

	selectableObject: storage

	Connections {
		target: storage

		function onDataChanged() {
			if (editor)
				_info = editor.storageInfo(storage)
		}
	}

	rightSourceComponent: Row {
		QBanner {
			num: storage ? storage.objectiveCount : 0
			visible: num > 0
			color: Qaterial.Colors.cyan900
			anchors.verticalCenter: parent.verticalCenter
		}

		Qaterial.RoundButton {
			icon.source: Qaterial.Icons.delete_
			icon.color: Qaterial.Colors.red400

			visible: storage && storage.objectiveCount == 0

			anchors.verticalCenter: parent.verticalCenter
			onClicked: editor.storageRemove(storage)
		}

	}
}
