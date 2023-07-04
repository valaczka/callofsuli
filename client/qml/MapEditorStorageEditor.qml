import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: root

	closeQuestion: modified ? qsTr("Biztosan eldobod a módosításokat?") : ""

	title: _infoStorage.name !== undefined ? _infoStorage.name : qsTr("Adatbank")
	subtitle: editor ? editor.displayName : ""

	property MapEditorStorage storage: null
	readonly property MapEditor editor: storage && storage.map ? storage.map.mapEditor : null
	readonly property var _infoStorage: editor ? editor.storageInfo(storage) : {}

	property bool modified: false

	appBar.backButtonVisible: true
	appBar.rightComponent: Row {
		rightPadding: 7 * Qaterial.Style.pixelSizeRatio
		Qaterial.SquareButton {
			backgroundImplicitHeight: Qaterial.Style.toolButton.appBarButtonHeight

			icon.source: Qaterial.Icons.checkBold
			text: qsTr("Kész")
			display: AbstractButton.TextBesideIcon
			enabled: modified && editor && _storageLoader.item

			onClicked: {
				if (storage.storageid <= 0) {
					editor.storageAdd(storage, function() {
						_storageLoader.item.saveData()
					})
				} else {
					editor.storageModify(storage, function() {
						_storageLoader.item.saveData()
					})
				}

				modified = false
				Client.stackPop(root)
			}
		}
	}


	QScrollable {
		anchors.fill: parent

		QIconLabel {
			id: _label
			visible: storage

			topPadding: iconLabelItem.implicitHeight*0.5

			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			icon.source: _infoStorage.icon !== undefined ? _infoStorage.icon : ""
			icon.width: 2.2 * Qaterial.Style.pixelSize
			icon.height: 2.2 * Qaterial.Style.pixelSize

			font: Qaterial.Style.textTheme.headline4
			text: _infoStorage.name !== undefined ? _infoStorage.name : ""

			elide: Text.ElideRight

			color: Qaterial.Colors.green400
		}

		Loader {
			id: _storageLoader
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			onStatusChanged: {
				if (status == Loader.Ready)
					item.loadData()
			}
		}

	}

	function loadQmls() {
		if (!editor)
			return

		if (storage) {
			_storageLoader.setSource(editor.storageQml(storage), {
										 storage: storage,
										 objectiveEditor: root,
										 readOnly: false
									 })
		} else
			_storageLoader.setSource("")
	}

	function previewRefresh() {

	}

	onStorageChanged: {
		if (storage)
			loadQmls()
		else
			Client.stackPop(root)
	}

	onEditorChanged: if (editor)
						 loadQmls()
}
