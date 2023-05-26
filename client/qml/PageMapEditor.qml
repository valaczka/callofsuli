import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: root

	stackPopFunction: function() {
		if (swipeView.currentIndex > 0) {
			swipeView.decrementCurrentIndex()
			return false
		}

		return true
	}

	property MapEditor mapEditor: _editor
	property url loadFile: ""

	title: qsTr("Pályaszerkesztő")
	subtitle: mapEditor.displayName

	MapEditor {
		id: _editor

		onSaveRequest: {
			if (currentFileName() === "")
				_actionSaveAs.trigger()
		}
	}

	appBar.backButtonVisible: true
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

			onClicked: mapEditor.map ? _menuMap.open() : _menuEmpty.open()

			QMenu {
				id: _menuEmpty

				QMenuItem { action: _actionCreate }
				QMenuItem { action: _actionOpen }
			}

			QMenu {
				id: _menuMap

				QMenuItem { action: _actionSaveAs }
			}
		}
	}

	Qaterial.Banner
	{
		anchors.top: parent.top
		width: parent.width
		drawSeparator: true
		text: qsTr("Nincs betöltve pálya!")
		iconSource: Qaterial.Icons.desktopClassic
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
					overrideQuestion(file)
				else
					mapEditor.saveAs(file)
				Client.Utils.settingsSet("folder/mapEditor", modelFolder.toString())
			}

			folder: mapEditor.currentFolder()
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
			model.append({ text: qsTr("Küldetések"), source: Qaterial.Icons.trophyBroken, color: "pink" })
			model.append({ text: qsTr("Feladatcsoportok"), source: Qaterial.Icons.account, color: "green" })
			model.append({ text: qsTr("Adatbankok"), source: Qaterial.Icons.trophyBroken, color: "pink" })
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
		icon.source: Qaterial.Icons.folder
		onTriggered: Qaterial.DialogManager.openFromComponent(_cmpFileSaveAs)
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


	function overrideQuestion(file) {
		JS.questionDialog({
							  onAccepted: function()
							  {
								  mapEditor.saveAs(file)
							  },
							  text: qsTr("A fájl létezik. Felülírjuk?\n%1").arg(file),
							  title: qsTr("Mentés másként"),
							  iconSource: Qaterial.Icons.fileAlert
						  })
	}

}
