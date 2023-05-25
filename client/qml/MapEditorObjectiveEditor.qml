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

	stackPopFunction: function() {
		/*if (_campaignList.view.selectEnabled) {
			_campaignList.view.unselectAll()
			return false
		}

		if (swipeView.currentIndex > 0) {
			swipeView.setCurrentIndex(0)
			return false
		}*/

		return true
	}

	title: _infoObjective.name !== undefined ? _infoObjective.name : qsTr("Feladat")
	subtitle: editor ? editor.displayName : ""

	property MapEditorChapter chapter: null
	property MapEditorStorage storage: null
	property MapEditorObjective objective: null
	readonly property MapEditor editor: objective && objective.map ? objective.map.mapEditor : null
	readonly property var _infoObjective: editor ? editor.objectiveInfo(objective) : {}
	readonly property var _infoStorage: editor ? editor.storageInfo(storage) : {}

	property bool modified: false


	appBar.backButtonVisible: true
	appBar.rightComponent: Qaterial.SquareButton {
		backgroundImplicitHeight: Qaterial.Style.toolButton.appBarButtonHeight

		icon.source: Qaterial.Icons.checkBold
		text: qsTr("Kész")
		display: AbstractButton.TextBesideIcon
		enabled: modified && editor && chapter

		onClicked: {
			if (objective.uuid == "") {
				if (_objectiveLoader.item)
					_objectiveLoader.item.saveData()

				editor.objectiveAdd(chapter, objective, storage, function() {
					if (_storageLoader.item)
						_storageLoader.item.saveData()
				})
			} else if (_objectiveLoader.item) {
				if (_storageLoader.item && _storageLoader.item.modified) {
					editor.objectiveModify(objective, storage, function() {
						_storageLoader.item.saveData()
						_objectiveLoader.item.saveData()
					})
				} else {
					editor.objectiveModify(objective, null, function() {
						_objectiveLoader.item.saveData()
					})
				}
			}

			modified = false
			Client.stackPop(root)
		}
	}


	QScrollable {
		anchors.fill: parent

		Qaterial.IconLabel {
			visible: objective

			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			icon.source: _infoObjective.icon !== undefined ? _infoObjective.icon : ""
			//icon.color: Qaterial.
			icon.width: 2.2 * Qaterial.Style.pixelSize
			icon.height: 2.2 * Qaterial.Style.pixelSize

			font: Qaterial.Style.textTheme.headline4
			text: _infoObjective.name !== undefined ? _infoObjective.name : ""

			elide: Text.ElideRight
		}

		Qaterial.IconLabel {
			visible: storage

			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			horizontalAlignment: Text.AlignLeft
			text: _infoStorage.name !== undefined ? _infoStorage.name : ""
			elide: Text.ElideRight

			icon.source: _infoStorage.icon !== undefined ? _infoStorage.icon : ""
			//icon.color: Qaterial.
			icon.width: 2.2 * Qaterial.Style.pixelSize
			icon.height: 2.2 * Qaterial.Style.pixelSize

			font: Qaterial.Style.textTheme.headline6
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

		Item {
			width: parent.width
			height: 10
			visible: _separator.visible
		}

		Qaterial.HorizontalLineSeparator {
			id: _separator
			width: parent.width*0.75
			anchors.horizontalCenter: parent.horizontalCenter
			visible: _storageLoader.item && _objectiveLoader.item
		}

		Item {
			width: parent.width
			height: 10
			visible: _separator.visible
		}

		Loader {
			id: _objectiveLoader
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
										 objectiveEditor: root
									 })
		} else
			_storageLoader.setSource("")

		if (objective) {
			_objectiveLoader.setSource(editor.objectiveQml(objective), {
										   objective: objective,
										   storage: storage,
										   objectiveEditor: root
									   })
		}
		else
			_objectiveLoader.setSource("")
	}

	onObjectiveChanged: {
		if (objective)
			loadQmls()
		else
			Client.stackPop(root)
	}

	onStorageChanged: if (storage)
						  loadQmls()

	onEditorChanged: if (editor)
						 loadQmls()
}
