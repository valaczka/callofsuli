import QtQuick
import QtQuick.Controls
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPage {
	id: root

	closeQuestion: modified ? qsTr("Biztosan eldobod a módosításokat?") : ""

	title: chapter ? chapter.name : _infoObjective.name !== undefined ? _infoObjective.name : qsTr("Feladat")
	subtitle: editor ? editor.displayName : ""

	property MapEditorChapter chapter: null
	property MapEditorStorage storage: null
	property MapEditorObjective objective: null
	readonly property MapEditor editor: objective && objective.map ? objective.map.mapEditor : null
	readonly property var _infoObjective: editor ? editor.objectiveInfo(objective) : {}
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
	}



	QScrollable {
		anchors.fill: parent


		Row {
			visible: storage
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			Qaterial.IconLabel {
				width: parent.width-parent.spacing-_editButton.width

				anchors.verticalCenter: parent.verticalCenter

				horizontalAlignment: Text.AlignLeft
				text: _infoStorage.name !== undefined ? _infoStorage.name : ""
				elide: Text.ElideRight

				icon.source: _infoStorage.icon !== undefined ? _infoStorage.icon : ""

				icon.width: 2.2 * Qaterial.Style.pixelSize
				icon.height: 2.2 * Qaterial.Style.pixelSize

				font: Qaterial.Style.textTheme.headline6
				color: Qaterial.Colors.green400
			}


			Qaterial.RoundButton {
				id: _editButton
				icon.source: _storageLoader.item && !_storageLoader.item.readOnly ? Qaterial.Icons.pencilOff : Qaterial.Icons.pencil
				ToolTip.text: qsTr("Szerkesztés")
				enabled: _storageLoader.item
				onClicked: _storageLoader.item.readOnly = !_storageLoader.item.readOnly
				anchors.verticalCenter: parent.verticalCenter
			}
		}


		Loader {
			id: _storageLoader
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			onStatusChanged: {
				if (status == Loader.Ready) {
					item.loadData()
					previewRefresh()
				}
			}
		}


		QIconLabel {
			id: _label
			visible: objective

			topPadding: iconLabelItem.implicitHeight*0.5

			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			icon.source: _infoObjective.icon !== undefined ? _infoObjective.icon : ""
			//icon.color: Qaterial.
			icon.width: 2.2 * Qaterial.Style.pixelSize
			icon.height: 2.2 * Qaterial.Style.pixelSize

			font: Qaterial.Style.textTheme.headline4
			text: _infoObjective.name !== undefined ? _infoObjective.name : ""

			elide: Text.ElideRight

			color: Qaterial.Style.iconColor()
		}


		Loader {
			id: _objectiveLoader
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			onStatusChanged: {
				if (status == Loader.Ready) {
					item.loadData()
					previewRefresh()
				}
			}
		}

		Item {
			width: parent.width
			height: 50 * Qaterial.Style.pixelSizeRatio
		}

		MapEditorObjectivePreview {
			id: _preview
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			refreshFunc: function() {
				if (!editor)
					return ""

				let sData = {}
				let oData = {}

				if (_storageLoader.item)
					sData = _storageLoader.item.previewData()

				if (_objectiveLoader.item)
					oData = _objectiveLoader.item.previewData()

				return editor.objectivePreview(objective ? objective.module : "", oData,
											   storage ? storage.module : "", sData)
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

	function previewRefresh() {
		_preview.refresh()
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
