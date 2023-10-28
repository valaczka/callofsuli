import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: root

	closeQuestion: mapEditor && mapEditor.modified ? qsTr("Biztosan eldobod a módosításokat?") : ""

	stackPopFunction: function() {
		if (swipeView.currentIndex > 0) {
			swipeView.decrementCurrentIndex()
			return false
		}

		if (mapEditor) {
			let l = mapEditor.checkMap()

			if (l.length && !_errorShown) {
				Client.messageWarning(l.join("\n"), qsTr("A pálya hibákat tartalmaz"))
				_errorShown = true
				return false
			}
		}

		return true
	}

	property MapEditor mapEditor: _editor
	property url loadFile: ""
	property bool online: false

	property bool _errorShown: false

	title: qsTr("Pályaszerkesztő")
	subtitle: mapEditor.displayName

	MapEditor {
		id: _editor

		onSaveRequest: {
			if (currentFileName() === "")
				_actionSaveAs.trigger()
		}
	}

	Connections {
		target: mapEditor

		function onModifiedChanged() {
			if (mapEditor.modified)
				_errorShown = false
		}
	}

	appBar.backButtonVisible: StackView.index > 1
	appBar.rightComponent: MapEditorToolbarComponent {
		editor: mapEditor
		Shortcut {
			sequence: "Ctrl+S"
			onActivated: mapEditor.save()
			enabled: mapEditor.map
		}

		Shortcut {
			sequence: "Ctrl+Z"
			enabled: mapEditor.map
			onActivated: if (mapEditor.undoStack.canUndo)
							 mapEditor.undoStack.undo()
		}

		Shortcut {
			sequence: "Ctrl+Y"
			enabled: mapEditor.map
			onActivated: if (mapEditor.undoStack.canRedo)
							 mapEditor.undoStack.redo()
		}


		Qaterial.AppBarButton
		{
			icon.source: Qaterial.Icons.dotsVertical

			onClicked: mapEditor.map ? (online ? _menuOnline.open() : _menuMap.open()) : _menuEmpty.open()

			QMenu {
				id: _menuEmpty

				QMenuItem { action: _actionCreate }
				QMenuItem { action: _actionOpen }
			}

			QMenu {
				id: _menuMap

				QMenuItem { action: _actionSaveAs }
				QMenuItem { action: _actionSaveNew }
				Qaterial.MenuSeparator {}
				QMenuItem { action: _actionImport }
			}

			QMenu {
				id: _menuOnline

				QMenuItem { action: _actionPublish }
				QMenuItem { action: _actionDelete }
				Qaterial.MenuSeparator {}
				QMenuItem { action: _actionSaveAs }
				QMenuItem { action: _actionSaveNew }
			}
		}
	}

	Qaterial.Banner
	{
		anchors.top: parent.top
		width: parent.width
		drawSeparator: true
		text: qsTr("Nincs betöltve pálya!")
		iconSource: Qaterial.Icons.briefcaseOffOutline
		fillIcon: false
		outlinedIcon: true
		highlightedIcon: true

		action1: qsTr("Létrehozás")
		action2: qsTr("Megnyitás")

		onAction1Clicked: _actionCreate.trigger()
		onAction2Clicked: _actionOpen.trigger()

		visible: !mapEditor.map
	}

	Component {
		id: _cmpFileOpen

		QFileDialog {
			title: qsTr("Pálya megnyitása")
			filters: [ "*.map" ]
			onFileSelected: {
				if (mapEditor.hasBackup(file)) {
					backupQuestion(file)
				} else {
					mapEditor.openFile(file)
				}
				Client.Utils.settingsSet("folder/mapEditor", modelFolder.toString())
			}

			folder: Client.Utils.settingsGet("folder/mapEditor", "")
		}

	}

	Component {
		id: _cmpFileSaveAs

		QFileDialog {
			title: qsTr("Pálya mentése másként")
			filters: [ "*.map" ]
			isSave: true
			suffix: ".map"
			onFileSelected: {
				if (Client.Utils.fileExists(file))
					overrideQuestion(file, false)
				else
					mapEditor.saveAs(file, false)
				Client.Utils.settingsSet("folder/mapEditor", modelFolder.toString())
			}

			folder: mapEditor.currentFolder()
		}
	}

	Component {
		id: _cmpFileSaveNew

		QFileDialog {
			title: qsTr("Pálya mentése azonosítók cseréjével")
			filters: [ "*.map" ]
			isSave: true
			suffix: ".map"
			onFileSelected: {
				if (Client.Utils.fileExists(file))
					overrideQuestion(file, true)
				else
					mapEditor.saveAs(file, true)
				Client.Utils.settingsSet("folder/mapEditor", modelFolder.toString())
			}

			folder: mapEditor.currentFolder()
		}
	}


	Component {
		id: _cmpFileImport

		QFileDialog {
			title: qsTr("Importálás")
			filters: [ "*.map" ]
			onFileSelected: {
				mapEditor.chapterImport(file)
				Client.Utils.settingsSet("folder/mapEditor", modelFolder.toString())
			}

			folder: Client.Utils.settingsGet("folder/mapEditor", "")
		}

	}



	Qaterial.SwipeView
	{
		id: swipeView
		anchors.fill: parent
		currentIndex: tabBar.currentIndex

		visible: mapEditor.map


		MapEditorMissionList {
			editor: mapEditor
		}


		MapEditorChapterList {
			editor: mapEditor
		}


		MapEditorStorageList {
			editor: mapEditor
		}
	}

	footer: QTabBar {
		id: tabBar
		currentIndex: swipeView.currentIndex

		visible: mapEditor.map

		Component.onCompleted: {
			model.append({ text: qsTr("Küldetések"), source: Qaterial.Icons.trophy, color: Qaterial.Colors.amber200 })
			model.append({ text: qsTr("Feladatcsoportok"), source: Qaterial.Icons.folderMultiple, color: Qaterial.Colors.purple100 })
			model.append({ text: qsTr("Adatbankok"), source: Qaterial.Icons.database, color: Qaterial.Colors.green300 })
		}
	}


	MapEditorObjectiveDialog {
		editor: mapEditor
	}

	onLoadFileChanged: if (loadFile.toString() != "")
						   mapEditor.openFile(loadFile)


	Action {
		id: _actionCreate
		text: qsTr("Létrehozás")
		icon.source: Qaterial.Icons.filePlus
		onTriggered: mapEditor.createFile()
	}

	Action {
		id: _actionOpen
		text: qsTr("Megnyitás")
		icon.source: Qaterial.Icons.folder
		onTriggered: Qaterial.DialogManager.openFromComponent(_cmpFileOpen)
	}

	Action {
		id: _actionSaveAs
		text: qsTr("Mentés másként")
		icon.source: Qaterial.Icons.contentSaveEdit
		onTriggered: {
			if (Qt.platform.os == "wasm")
				mapEditor.wasmSaveAs(false)
			else
				Qaterial.DialogManager.openFromComponent(_cmpFileSaveAs)
		}
	}

	Action {
		id: _actionSaveNew
		text: qsTr("Mentés újként")
		icon.source: Qaterial.Icons.contentSaveMoveOutline
		onTriggered: {
			if (Qt.platform.os == "wasm")
				mapEditor.wasmSaveAs(true)
			else
				Qaterial.DialogManager.openFromComponent(_cmpFileSaveNew)
		}
	}


	Action {
		id: _actionImport
		text: qsTr("Importálás")
		icon.source: Qaterial.Icons.fileImport
		onTriggered: {
			if (Qt.platform.os == "wasm")
				mapEditor.chapterImport("")
			else
				Qaterial.DialogManager.openFromComponent(_cmpFileImport)
		}
	}

	Action {
		id: _actionPublish
		text: qsTr("Közzététel")
		icon.source: Qaterial.Icons.send
		enabled: mapEditor && !mapEditor.modified
		onTriggered: JS.questionDialog(
						 {
							 onAccepted: function()
							 {
								 publishDraft()
							 },
							 text: qsTr("Biztosan közzéteszed a vázlatot?"),
							 title: qsTr("Közzététel"),
							 iconSource: Qaterial.Icons.send
						 })

	}

	Action {
		id: _actionDelete
		text: qsTr("Vázlat törlése")
		icon.source: Qaterial.Icons.cancel
		enabled: mapEditor && !mapEditor.modified
		onTriggered: JS.questionDialog(
						 {
							 onAccepted: function()
							 {
								 deleteDraft()
							 },
							 text: qsTr("Biztosan törlöd a vázlatot?"),
							 title: qsTr("Vázlat törlése"),
							 iconSource: Qaterial.Icons.deleteAlert
						 })
	}

	function backupQuestion(file) {
		Qaterial.DialogManager.showDialog(
					{
						onAccepted: function()
						{
							mapEditor.openFile(file, true)
						},
						onNo: function()
						{
							mapEditor.openFile(file, false)
						},
						text: qsTr("Automatikus mentés található. Visszaállítsuk?\n%1").arg(file),
						title: qsTr("Visszaállítás mentésből"),
						iconSource: Qaterial.Icons.fileAlert,
						iconColor: Qaterial.Colors.orange500,
						textColor: Qaterial.Colors.orange500,
						iconFill: false,
						iconSize: Qaterial.Style.roundIcon.size,
						standardButtons: Dialog.No | Dialog.Yes | Dialog.Cancel
					})
	}


	function overrideQuestion(file, isNew) {
		JS.questionDialog({
							  onAccepted: function()
							  {
								  mapEditor.saveAs(file, isNew)
							  },
							  text: qsTr("A fájl létezik. Felülírjuk?\n%1").arg(file),
							  title: qsTr("Mentés másként"),
							  iconSource: Qaterial.Icons.fileAlert
						  })
	}

	function publishDraft() {
		let uuid = mapEditor.uuid
		let version = mapEditor.draftVersion
		Client.send(WebSocket.ApiTeacher, "map/%1/publish/%2".arg(uuid).arg(version))
		.done(root, function(r){
			Client.messageInfo(qsTr("Sikeres közzététel"), mapEditor.displayName)
			Client.stackPop(root)
		})
		.fail(root, JS.failMessage("Közzététel sikertelen"))
	}

	function deleteDraft() {
		let uuid = mapEditor.uuid
		let version = mapEditor.draftVersion
		Client.send(WebSocket.ApiTeacher, "map/%1/deleteDraft/%2".arg(uuid).arg(version))
		.done(root, function(r){
			Client.messageInfo(qsTr("Vázlat törlése sikeres"), mapEditor.displayName)
			Client.stackPop(root)
		})
		.fail(root, JS.failMessage("Törlés sikertelen"))
	}
}
