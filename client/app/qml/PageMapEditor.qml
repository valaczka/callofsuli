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

	title: mapEditor.displayName

	toolbarLoaderComponent: Row {
		QToolButton {
			action: actionUndo
			anchors.verticalCenter: parent.verticalCenter
			visible: !_isSmall
		}
		QToolButton {
			action: actionRedo
			anchors.verticalCenter: parent.verticalCenter
			visible: !_isSmall
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
		MenuItem { action: actionClose }
	}

	QMenu {
		id: menuSmall
		MenuItem { action: actionUndo; text: qsTr("Visszavonás") }
		MenuItem { action: actionRedo; text: qsTr("Ismét") }
		MenuSeparator {}
		MenuItem { action: actionOpen }
		MenuItem { action: actionSaveAs }
		MenuItem { action: actionClose }
	}


	activity: MapEditor {
		id: mapEditor

		onEditorChanged: if (editor) {
							 buttonModel = modelMain
							 loadComponent()
						 } else {
							 replaceContent(componentMain)
							 buttonModel = null
						 }

		onActionContextUpdated: {
			console.debug("UPDATED", type, contextId)
			loadComponent(type, contextId)
		}
	}

	buttonModel: null

	ListModel {
		id: modelMain

		ListElement {
			title: qsTr("Küldetések")
			icon: "image://font/Academic/\uf207"
			//iconColor: "chartreuse"
			func: function() { replaceContent(componentMissions) }
			checked: true
		}
		ListElement {
			title: qsTr("Feladatok")
			icon: "image://font/Academic/\uf207"
			//iconColor: "chartreuse"
			func: function() { replaceContent(componentChapters) }
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


	Component {
		id: componentTest
		Item {
			implicitWidth: 200
			implicitHeight: 200

			//property int contextAction: (MapEditorAction.ActionTypeChapterList | MapEditorAction.ActionTypeChapter)

			Column {
				anchors.centerIn: parent
				QLabel {
					text: "Step: %1 / %2\nSaved: %3".arg(mapEditor.undoStack.step).arg(mapEditor.undoStack.size).arg(mapEditor.undoStack.savedStep)
				}
				QButton {
					text: "add"
					onClicked: mapEditor.addTest()
				}
				QButton {
					text: "remove"
					onClicked: mapEditor.removeTest()
				}
			}
		}
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


			Column {
				anchors.centerIn: parent
				QToolButtonBig {
					action: actionNew
				}
				QToolButtonBig {
					action: actionOpen
				}
			}
		}
	}


	Component {
		id: componentRegistration
		Registration {}
	}




	Action {
		id: actionUndo
		icon.source: CosStyle.iconUndo
		enabled: mapEditor.editor && mapEditor.undoStack.canUndo
		onTriggered: mapEditor.undoStack.undo()
	}


	Action {
		id: actionRedo
		icon.source: CosStyle.iconSend
		enabled: mapEditor.editor && mapEditor.undoStack.canRedo
		onTriggered: mapEditor.undoStack.redo()
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
		onTriggered: mapEditor.open("file:///home/valaczka/ddd.map")
	}


	Action {
		id: actionNew
		icon.source: CosStyle.iconAdd
		text: qsTr("Új pálya")
		enabled: !mapEditor.editor
		onTriggered: mapEditor.create()
	}


	Action {
		id: actionClose
		icon.source: CosStyle.iconClose
		text: qsTr("Bezárás")
		enabled: mapEditor.editor
		onTriggered: mapEditor.close()
	}

	Action {
		id: actionSaveAs
		icon.source: CosStyle.iconSearch
		text: qsTr("Mentés másként")
		enabled: mapEditor.editor
		//onTriggered: mapEditor.undoStack.undo()
	}

	/*function checkRoles() {
		if (cosClient.userRoles & Client.RoleAdmin) {
			if (buttonModel !== modelAdmin)
				replaceContent()
			buttonModel = modelAdmin
			title = ""
			buttonColor = CosStyle.colorPrimaryLight
			buttonBackgroundColor = "#5e0000"
		} else if (cosClient.userRoles & Client.RoleTeacher) {
			if (buttonModel !== modelTeacher)
				replaceContent()
			buttonModel = modelTeacher
			title = ""
			buttonColor = CosStyle.colorPrimaryLighter
			buttonBackgroundColor = "#33220c"
		} else if (cosClient.userRoles & Client.RoleStudent) {
			if (buttonModel !== modelStudent)
				replaceContent()
			buttonModel = modelStudent
			title = ""
			buttonColor = CosStyle.colorPrimary
			buttonBackgroundColor = "#111147"
		} else {
			if (buttonModel !== modelGuest)
				replaceContent()
			buttonModel = modelGuest
			title = cosClient.serverName
			buttonColor = CosStyle.colorPrimaryDark
			buttonBackgroundColor = "black"
		}
	}*/



	Component.onCompleted: replaceContent(componentMain)


	function loadComponent(contextAction, contextId) {
		if (!mapEditor.editor)
			return

		console.debug("LOAD COMPONENT", contextAction, contextId, stack.currentItem.contextAction)

		if (stack.currentItem && stack.currentItem.contextAction && (stack.currentItem.contextAction & contextAction)) {
			stack.currentItem.loadContextId(contextId)
		} else {
			var cmp = componentTest

			switch (contextAction) {
			case MapEditorAction.ActionTypeChapterList:
			case MapEditorAction.ActionTypeChapter:
				cmp = componentMissions
				break
			}

			console.debug("**** REPLACE CONTENT", cmp, contextAction, contextId)

			replaceContent(cmp, {contextId: contextId})
		}
	}




	function loadContextId(id) {
		console.debug("MAIN LOAD", id)
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
