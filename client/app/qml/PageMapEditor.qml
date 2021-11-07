import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QBasePage {
	id: control

	defaultTitle: qsTr("Pályaszerkesztő")
	defaultSubTitle: mapEditor.loaded ? mapEditor.readableFilename : ""

	property url fileToOpen: ""

	activity: MapEditor {
		id: mapEditor

		property Drawer drawer: mapEditorDrawer

		onLoadStarted: {
			var d = JS.dialogCreateQml("Progress", { title: qsTr("Betöltés"), text: filename })
			d.closePolicy = Popup.NoAutoClose

			d.rejected.connect(function(data) {
				mapEditor.loadAbort()
			})

			mapEditor.loadProgressChanged.connect(d.item.setValue)
			mapEditor.loadSucceed.connect(d.item.dlgClose)
			mapEditor.loadFailed.connect(d.item.dlgClose)

			d.onClosedAndDestroyed.connect(function() {
				mapEditor.loadProgressChanged.disconnect(d.item.setValue)
				mapEditor.loadSucceed.disconnect(d.item.dlgClose)
				mapEditor.loadFailed.disconnect(d.item.dlgClose)
			})

			d.open()
		}

		onLoadSucceed: {
			getMissionList()
			getChapterList()
			getObjectiveList()
			getStorageList()
			actionMissions.trigger()
		}

		onSaveSucceed: {
			if (isCopy)
				cosClient.sendMessageInfo(qsTr("Másolat készítése sikerült"), filename)
		}

		onSaveDialogRequest: {
			if (!mapEditor.loaded)
				return

			var d = JS.dialogCreateQml("File", {
										   isSave: true,
										   folder: cosClient.getSetting("mapFolder", "")
									   })

			d.accepted.connect(function(data){
				if (!isNew)
					mapEditor.saveCopyUrl(data)
				else
					mapEditor.saveUrl(data)
				cosClient.setSetting("mapFolder", d.item.modelFolder)
			})

			d.open()
		}

		onPlayFailed: {
			cosClient.sendMessageError(qsTr("Lejátszási hiba"), error)
		}

		onPlayReady: {
			var o = JS.createPage("Game", {
									  gameMatch: gamematch,
									  deleteGameMatch: true
								  })

		}





		onChapterMissionListReady: {
			if (data.missions.length === 0) {
				cosClient.sendMessageWarning(qsTr("Küldetések"), qsTr("Még nincsen egyetlen küldetés sem!"))
				return
			}


			mapEditor.modelDialogChapterMissionList.replaceList(data.missions)
			mapEditor.modelDialogChapterMissionList.selectByRole("used")

			var d = JS.dialogCreateQml("MissionList", {
										   icon: CosStyle.iconLockAdd,
										   title: data.chapterName.length ? data.chapterName : qsTr("Szakaszhoz tartozó küldetések"),
										   selectorSet: true,
										   sourceModel: mapEditor.modelDialogChapterMissionList
									   })


			d.accepted.connect(function(dlgdata) {
				if (dlgdata !== true)
					return

				var l = mapEditor.modelDialogChapterMissionList.getSelectedData(["uuid", "level"])

				mapEditor.chapterMissionListModify(data.chapter, mapEditor.modelDialogChapterMissionList, "used")

			})
			d.open()

		}



		onChapterImportReady: {
			var d = JS.dialogCreateQml("YesNo", {text: qsTr("%1 tétel importálható?").arg(records.length)})
			d.accepted.connect(function() {
				mapEditor.objectiveImport({list: records})
			})
			d.open()
			return true
		}

	}

	toolBarMenu: QMenu {
		enabled: mapEditor.loaded

		MenuItem {
			action: actionMissions
		}
		MenuItem {
			action: actionChapters
		}
	}

	mainToolBarComponent: Row {
		spacing: 0
		visible: mapEditor.loaded
		QUndoButton {
			anchors.verticalCenter: parent.verticalCenter
			activity: mapEditor
		}
		QToolButton {
			anchors.verticalCenter: parent.verticalCenter
			action: actionSave
		}
	}


	mainMenuFunc: function (m) {
		m.addAction(actionSaveAs)
		if (mapEditor.isWithGraphviz)
			m.addAction(actionGraph)
	}

	Loader {
		id: editorLoader
		anchors.fill: parent
		enabled: !mapEditorDrawer.opened
	}



	QLabel {
		id: labelPermissions
		color: CosStyle.colorWarning
		anchors.centerIn: parent
		anchors.margins: 10
		text: qsTr("Jogosultságok ellenőrzése")
	}


	Column {
		visible: !labelPermissions.visible && editorLoader.status != Loader.Ready && !mapEditor.loaded

		anchors.centerIn: parent
		spacing: 5
		QToolButtonBig {
			anchors.horizontalCenter: parent.horizontalCenter
			action: actionCreate
		}
		QToolButtonBig {
			anchors.horizontalCenter: parent.horizontalCenter
			action: actionOpen
		}
	}

	Rectangle {
		anchors.fill: parent
		color: "black"
		opacity: 0.7 * mapEditorDrawer.position
		visible: mapEditorDrawer.position
	}


	QDrawer {
		id: mapEditorDrawer
		edge: Qt.BottomEdge
		height: control.height - control.mainToolBar.height
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




	Component {
		id: componentMissions
		MapEditorMissions {
		}
	}


	Component {
		id: componentChapters
		MapEditorChapterList {
		}
	}



	Action {
		id: actionCreate
		text: qsTr("Új")
		icon.source: CosStyle.iconAdd
		enabled: !mapEditor.loaded
		onTriggered: {
			mapEditor.create()
		}
	}

	Action {
		id: actionOpen
		text: qsTr("Megnyitás")
		enabled: !mapEditor.loaded
		icon.source: CosStyle.iconAdjust
		onTriggered: {
			var d = JS.dialogCreateQml("File", {
										   isSave: false,
										   folder: cosClient.getSetting("mapFolder", "")
									   })

			d.accepted.connect(function(data){
				mapEditor.openUrl(data)
				cosClient.setSetting("mapFolder", d.item.modelFolder)
			})

			d.open()
		}
	}


	Action {
		id: actionMissions
		text: qsTr("Küldetések")
		icon.source: CosStyle.iconAdd
		onTriggered: {
			control.title = qsTr("Küldetések")
			editorLoader.sourceComponent = componentMissions
		}
	}

	Action {
		id: actionChapters
		text: qsTr("Szakaszok")
		icon.source: CosStyle.iconAdd
		onTriggered: {
			control.title = qsTr("Szakaszok")
			editorLoader.sourceComponent = componentChapters
		}
	}



	Action {
		id: actionGraph
		icon.source: CosStyle.iconAdd
		text: qsTr("Folyamatábra")
		enabled: mapEditor.loaded
	}

	Action {
		id: actionSave
		icon.source: CosStyle.iconSave
		enabled: mapEditor.loaded && mapEditor.modified
		shortcut: "Ctrl+S"
		onTriggered: {
			mapEditor.save()
		}
	}

	Action {
		id: actionSaveAs
		icon.source: CosStyle.iconSave
		text: qsTr("Másolat")
		enabled: mapEditor.loaded
		onTriggered: {
			mapEditor.saveDialogRequest(false)
		}
	}

	onPageActivatedFirst: cosClient.checkPermissions()


	Connections {
		target: cosClient

		function onStoragePermissionsDenied() {
			labelPermissions.text = qsTr("Írási/olvasási jogosultság hiányzik")
			labelPermissions.color = CosStyle.colorErrorLighter
		}

		function onStoragePermissionsGranted() {
			labelPermissions.visible = false

			if (fileToOpen != "") {
				mapEditor.openUrl(fileToOpen)
			}
		}

	}


	function windowClose() {
		if (mapEditor.modified) {
			var d = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan eldobod a módosításokat?")})
			d.accepted.connect(function() {
				mapEditor.modified = false
				mainWindow.close()
			})
			d.open()
			return true
		}

		return false
	}

	function pageStackBack() {
		if (editorLoader.status == Loader.Ready && editorLoader.item.layoutBack())
			return true

		if (mapEditor.modified) {
			var d = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan eldobod a módosításokat?")})
			d.accepted.connect(function() {
				mapEditor.modified = false
				mainStack.back()
			})
			d.open()
			return true
		}


		return false
	}
}

