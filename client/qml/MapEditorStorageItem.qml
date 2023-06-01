import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

QIconLoaderItemDelegate {
	id: root

	property MapEditorStorage storage: null
	readonly property MapEditor editor: storage && storage.map ? storage.map.mapEditor : null
	property var _info: editor ? editor.storageInfo(storage) : {}

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
			icon.source: Qaterial.Icons._delete
			icon.color: Qaterial.Colors.red400

			visible: storage && storage.objectiveCount == 0

			anchors.verticalCenter: parent.verticalCenter
			onClicked: editor.storageRemove(storage)
		}

	}
}
