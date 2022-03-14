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

	property int _actionContextType: -1
	property var _actionContextId: null

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
		MenuItem { action: actionOpen }
		MenuItem { action: actionSaveAs }
	}

	QMenu {
		id: menuSmall
		MenuItem { action: actionUndo; text: qsTr("Visszavonás") }
		MenuItem { action: actionRedo; text: qsTr("Ismét") }
		MenuSeparator {}
		MenuItem { action: actionOpen }
		MenuItem { action: actionSaveAs }
	}


	activity: MapEditor {
		id: mapEditor

		property Drawer drawer: mapEditorDrawer

		onEditorChanged: if (editor) {
							 loadComponent()
						 } else {
							 replaceContent(componentMain)
						 }

		onActionContextUpdated: loadComponent(type, contextId)
	}


	tabBarVisible: mapEditor.editor

	buttonModel: ListModel {
		ListElement {
			title: qsTr("Küldetések")
			icon: "image://font/Academic/\uf207"
			//iconColor: "chartreuse"
			func: function() { loadComponentReal(componentMissions) }
			checked: false
		}
		ListElement {
			title: qsTr("Feladatok")
			icon: "image://font/Academic/\uf207"
			//iconColor: "chartreuse"
			func: function() { loadComponentReal(componentChapters) }
		}
		ListElement {
			title: qsTr("Storages")
			icon: "image://font/Academic/\uf207"
			//iconColor: "chartreuse"
			func: function() { replaceContent(null) }
		}
		ListElement {
			title: qsTr("Képek")
			icon: "image://font/Academic/\uf207"
			//iconColor: "chartreuse"
			func: function() { replaceContent(null) }
		}

	}


	enum Components {
		Missions = 0,
		Chapters,
		Storages,
		Images
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




	QDrawer {
		id: mapEditorDrawer
		edge: Qt.BottomEdge
		height: control.height - control.stack.y - control.headerPadding
		width: control.width>control.height ? Math.min(control.width*0.9, 1080) : control.width
		x: (control.width-width)/2

		property alias loader: drawerLoader

		dim: false
		interactive: false
		modal: true

		onClosed: {
			interactive = false
			drawerLoader.sourceComponent = undefined
		}

		onOpened: {
			interactive = true
		}


		Loader {
			id: drawerLoader
			anchors.fill: parent
			property Drawer drawer: mapEditorDrawer
			property MapEditor mapEditor: mapEditor

			onStatusChanged: if (drawerLoader.status == Loader.Ready)
								 mapEditorDrawer.open()
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
		icon.source: CosStyle.iconSend
		enabled: mapEditor.editor && mapEditor.undoStack.canRedo
		onTriggered: mapEditor.undoStack.redo()
		shortcut: "Ctrl+Shitf+Z"
	}


	Action {
		id: actionSave
		icon.source: CosStyle.iconSave
		enabled: mapEditor.editor && mapEditor.undoStack.savedStep !== mapEditor.undoStack.step
		//onTriggered: mapEditor.undoStack.undo()
	}


	Action {
		id: actionOpen
		icon.source: CosStyle.iconPreferences
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
		icon.source: CosStyle.iconAdd
		text: qsTr("Új pálya")
		enabled: !mapEditor.editor
		onTriggered: mapEditor.create()
	}


	Action {
		id: actionSaveAs
		icon.source: CosStyle.iconSearch
		text: qsTr("Mentés másként")
		enabled: mapEditor.editor
		//onTriggered: mapEditor.undoStack.undo()
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

			if (fileToOpen != "") {
				mapEditor.open(fileToOpen)
			}
		}

	}


	function loadComponent(contextAction, contextId) {
		if (!mapEditor.editor)
			return

		if (stack.currentItem && stack.currentItem.contextAction && (stack.currentItem.contextAction & contextAction)) {
			stack.currentItem.loadContextId(contextAction, contextId)
		} else {
			var cmp = PageMapEditor.Components.Missions

			switch (contextAction) {
			case MapEditorAction.ActionTypeChapterList:
			case MapEditorAction.ActionTypeChapter:
				cmp = PageMapEditor.Components.Chapters
				break
			}

			_actionContextType = contextAction ? contextAction : -1
			_actionContextId = contextId ? contextId : null

			activateButton(cmp)
		}
	}


	function loadComponentReal(cmp) {
		replaceContent(cmp, {
						   actionContextType: _actionContextType,
						   actionContextId: _actionContextId
					   })
		_actionContextType = -1
		_actionContextId = null
	}



	function loadContextId(type, id) {
		console.debug("MAIN LOAD", type, id)
	}


	pageBackCallbackFunction: function () {
		if (_closeEnabled || !mapEditor.editor || !actionSave.enabled)
			return false

		var d = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan bezárod mentés nélkül?\n%1").arg(mapEditor.displayName)})
		d.accepted.connect(function() {
			_closeEnabled = true
			mainStack.back()
		})
		d.open()
		return true
	}

	property var closeCallbackFunction: function () {
		if (_closeEnabled || !mapEditor.editor || !actionSave.enabled)
			return false

		var d = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan bezárod mentés nélkül?\n%1").arg(mapEditor.displayName)})
		d.accepted.connect(function() {
			_closeEnabled = true
			mainWindow.close()
		})
		d.open()
		return true
	}
}
