import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QTabPage {
	id: control

	property bool _closeEnabled: false
	property bool _isSmall: width < 800
	property bool _permissionsGranted: false
	property bool _permissionsDenied: false

	property url fileToOpen: ""

	title: mapEditor.displayName

	toolBarLoaderComponent: Row {
		QToolButton {
			action: actionUndo
			anchors.verticalCenter: parent.verticalCenter
			visible: !_isSmall
			text: qsTr("Vissza: %1").arg(mapEditor.undoStack.undoText)
			display: AbstractButton.IconOnly
		}
		QToolButton {
			action: actionRedo
			anchors.verticalCenter: parent.verticalCenter
			visible: !_isSmall
			text: qsTr("Ismét: %1").arg(mapEditor.undoStack.redoText)
			display: AbstractButton.IconOnly
		}
		QToolButton {
			action: actionSave
			anchors.verticalCenter: parent.verticalCenter
		}
		QMenuButton {
			menu: _isSmall ? menuSmall : menuWide
			anchors.verticalCenter: parent.verticalCenter
		}
	}

	QMenu {
		id: menuWide
		//MenuItem { action: actionOpen }
		MenuItem { action: actionSaveAs }
	}

	QMenu {
		id: menuSmall
		MenuItem { action: actionUndo; text: qsTr("Visszavonás") }
		MenuItem { action: actionRedo; text: qsTr("Ismét") }
		MenuSeparator {}
		//MenuItem { action: actionOpen }
		MenuItem { action: actionSaveAs }
	}


	activity: MapEditor {
		id: mapEditor

		onEditorChanged: if (editor) {
							 activateButton(PageMapEditor.Components.Missions)
						 } else {
							 replaceContent(componentMain)
						 }

		onActionContextUpdated: loadComponent(type, contextId)

		function openObjective(params) {
			pushContent(componentObjective, params)
		}

		function openMissionLevel(params) {
			pushContent(componentMissionLevel, params)
		}

		onMissionLevelOpenRequest: pushContent(componentMissionLevel, {
												   missionLevel: missionLevel
											   })

		onMissionLevelRemoved: if (stack.currentItem.contextAction === MapEditorAction.ActionTypeMissionLevel)
								   stack.pop()

		onGamePlayReady: {
			var o = JS.createPage("Game", {
									  gameMatch: gameMatch,
									  deleteGameMatch: true
								  })
		}
	}


	tabBarVisible: mapEditor.editor

	buttonModel: ListModel {
		ListElement {
			title: qsTr("Küldetések")
			icon: "image://font/School/\uf19d"
			iconColor: "orange"
			func: function() { replaceContent(componentMissions) }
			checked: false
		}
		ListElement {
			title: qsTr("Feladatok")
			icon: "image://font/AcademicI/\uf170"
			iconColor: "chartreuse"
			func: function() { replaceContent(componentChapters) }
		}
		ListElement {
			title: qsTr("Adatbankok")
			icon: "image://font/Academic/\uf1a3"
			iconColor: "deepskyblue"
			func: function() { replaceContent(componentStorages) }
		}
		/*ListElement {
			title: qsTr("Képek")
			icon: "image://font/Academic/\uf207"
			//iconColor: "chartreuse"
			func: function() { replaceContent(null) }
		}*/

	}


	enum Components {
		Missions = 0,
		Chapters,
		Storages
		//Images
	}



	Component {
		id: componentMissions
		MapEditorMissionList {}
	}

	Component {
		id: componentChapters
		MapEditorChapterList {}
	}

	Component {
		id: componentObjective
		MapEditorObjective {
			_mapEditor: mapEditor
			onClose: stack.pop()
		}
	}

	Component {
		id: componentMissionLevel
		MapEditorMissionLevel { }
	}

	Component {
		id: componentStorages
		MapEditorStorageList {}
	}




	Component {
		id: componentMain
		Item {
			implicitWidth: 200
			implicitHeight: 200

			QLabel {
				id: labelPermissions
				color: _permissionsDenied ? CosStyle.colorErrorLighter : CosStyle.colorWarning
				anchors.centerIn: parent
				anchors.margins: 10
				text: _permissionsDenied ? qsTr("Írási/olvasási jogosultság hiányzik") : qsTr("Jogosultságok ellenőrzése")
				visible: !_permissionsGranted
			}


			Column {
				visible: _permissionsGranted
				anchors.centerIn: parent
				spacing: 5
				QToolButtonBig {
					anchors.horizontalCenter: parent.horizontalCenter
					action: actionNew
				}
				QToolButtonBig {
					anchors.horizontalCenter: parent.horizontalCenter
					action: actionOpen
				}
			}
		}
	}





	Action {
		id: actionUndo
		icon.source: CosStyle.iconUndo
		enabled: mapEditor.editor && mapEditor.undoStack.canUndo
		onTriggered: mapEditor.undoStack.undo()
		shortcut: "Ctrl+Z"
	}


	Action {
		id: actionRedo
		icon.source: CosStyle.iconRedo
		enabled: mapEditor.editor && mapEditor.undoStack.canRedo
		onTriggered: mapEditor.undoStack.redo()
		shortcut: "Ctrl+Shift+Z"
	}


	Action {
		id: actionSave
		icon.source: CosStyle.iconSave
		enabled: mapEditor.editor && mapEditor.undoStack.savedStep !== mapEditor.undoStack.step
		shortcut: "Ctrl+S"
		onTriggered: {
			if (mapEditor.url.toString() != "")
				mapEditor.save()
			else
				actionSaveAs.trigger()
		}
	}


	Action {
		id: actionOpen
		icon.source: CosStyle.iconOpen
		text: qsTr("Megnyitás")
		enabled: !mapEditor.editor
		onTriggered: {
			var d = JS.dialogCreateQml("File", {
										   isSave: false,
										   folder: cosClient.getSetting("mapFolder", "")
									   })

			d.accepted.connect(function(data){
				mapEditor.open(data)
				cosClient.setSetting("mapFolder", d.item.modelFolder)
			})

			d.open()
		}
	}


	Action {
		id: actionNew
		icon.source: CosStyle.iconNew
		text: qsTr("Új pálya")
		enabled: !mapEditor.editor
		onTriggered: mapEditor.create()
	}


	Action {
		id: actionSaveAs
		icon.source: "image://font/Material Icons/\ue02e"
		text: qsTr("Mentés másként")
		enabled: mapEditor.editor
		onTriggered:  {
			var d = JS.dialogCreateQml("File", {
										   isSave: true,
										   folder: cosClient.getSetting("mapFolder", "")
									   })

			d.accepted.connect(function(data){
				mapEditor.save(data)
				cosClient.setSetting("mapFolder", d.item.modelFolder)
			})

			d.open()
		}
	}



	Component.onCompleted: replaceContent(componentMain)

	onPageActivatedFirst: cosClient.checkPermissions()


	Connections {
		target: cosClient

		function onStoragePermissionsDenied() {
			_permissionsDenied = true
		}

		function onStoragePermissionsGranted() {
			_permissionsGranted = true

			if (!mapEditor.editor && fileToOpen != "") {
				mapEditor.open(fileToOpen)
				fileToOpen = ""
			}
		}

	}




	function loadComponent(contextAction, contextId) {
		if (!mapEditor.editor)
			return

		if (stack.currentItem.contextAction === MapEditorAction.ActionTypeObjective &&
				contextAction !== MapEditorAction.ActionTypeObjective)
			activateButton(PageMapEditor.Components.Chapters)
		else if (stack.currentItem.contextAction === MapEditorAction.ActionTypeMissionLevel &&
				 contextAction !== MapEditorAction.ActionTypeMissionLevel && contextAction !== MapEditorAction.ActionTypeInventory)
			activateButton(PageMapEditor.Components.Missions)
		else if (stack.currentItem.contextAction === MapEditorAction.ActionTypeObjective &&
				 contextAction === MapEditorAction.ActionTypeObjective)
			stack.currentItem.loadContextId(contextId)
	}




	pageBackCallbackFunction: function () {
		if (_closeEnabled || !mapEditor.editor)
			return false

		if (actionSave.enabled) {
			var d = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan bezárod mentés nélkül?\n%1").arg(mapEditor.displayName)})
			d.accepted.connect(function() {
				_closeEnabled = true
				mainStack.back()
			})
			d.open()
			return true
		}

		var err = mapEditor.checkMap()

		if (err !== "") {
			var dd = JS.dialogCreateQml("YesNoFlickable", {
											title: qsTr("Hibás pálya"),
											text: qsTr("A pálya hibákat tartalmaz.\nEnnek ellenére bezárod a szerkesztőt?"),
											details: err
										})

			dd.item.titleColor = CosStyle.colorWarningLighter
			dd.item.textColor = CosStyle.colorWarningLight

			dd.accepted.connect(function() {
				_closeEnabled = true
				mainStack.back()
			})

			dd.open()
			return true
		}

		return false
	}

	closeCallbackFunction: function () {
		if (windowCloseFunction())
			return true

		if (_closeEnabled || !mapEditor.editor)
			return false

		if (actionSave.enabled) {
			var d = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan bezárod mentés nélkül?\n%1").arg(mapEditor.displayName)})
			d.accepted.connect(function() {
				_closeEnabled = true
				mainWindow.close()
			})
			d.open()
			return true
		}

		var err = mapEditor.checkMap()

		if (err !== "") {
			var dd = JS.dialogCreateQml("YesNoFlickable", {
											title: qsTr("Hibás pálya"),
											text: qsTr("A pálya hibákat tartalmaz.\nEnnek ellenére bezárod a szerkesztőt?"),
											details: err
										})

			dd.item.titleColor = CosStyle.colorWarningLighter
			dd.item.textColor = CosStyle.colorWarningLight

			dd.accepted.connect(function() {
				_closeEnabled = true
				mainWindow.close()
			})

			dd.open()
			return true
		}


		return false
	}
}
