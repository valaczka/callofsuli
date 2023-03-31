import QtQuick 2.12
import QtQuick.Controls 2.12
import SortFilterProxyModel 0.2
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: control

	stackPopFunction: function() {
		if (view.selectEnabled) {
			view.unselectAll()
			return false
		}

		return true
	}

	title: qsTr("Pályák")
	subtitle: Client.server ? Client.server.serverName : ""

	appBar.backButtonVisible: true
	appBar.rightComponent: Qaterial.AppBarButton
	{
		icon.source: Qaterial.Icons.dotsVertical
		onClicked: menu.open()

		QMenu {
			id: menu

			QMenuItem { action: actionMapImport }
			Qaterial.MenuSeparator {}
			QMenuItem { action: actionMapAdd }
			QMenuItem { action: actionMapRename }
			QMenuItem { action: actionMapRemove }
		}
	}


	TeacherMapHandler {
		id: handler
	}

	QListView {
		id: view

		currentIndex: -1
		anchors.fill: parent
		autoSelectChange: true

		refreshProgressVisible: Client.webSocket.pending
		refreshEnabled: true
		onRefreshRequest: reload()

		model: SortFilterProxyModel {
			sourceModel: handler.mapList

			sorters: [
				StringSorter {
					roleName: "name"
					sortOrder: Qt.AscendingOrder
				}
			]
		}

		delegate: QIconLoaderItemDelegate {
			id: item

			property TeacherMap mapObject: model.qtObject
			selectableObject: mapObject

			highlighted: ListView.isCurrentItem
			iconSource: Qaterial.Icons.desktopClassic
			text: mapObject ? mapObject.name : ""
			secondaryText: mapObject ? qsTr("%1. verzió (%2 @%3)").arg(mapObject.version)
									   .arg(mapObject.lastModified.toLocaleString(Qt.locale(), "yyyy. MMM d. H:mm:ss"))
									   .arg(mapObject.lastEditor)
									 : ""

			rightSourceComponent: QDownloadProgressIcon {
				map: mapObject
			}

			onClicked: if (mapObject && mapObject.downloaded)
						   console.debug("ok")
					   else
						   handler.mapDownload(mapObject)
		}


		Qaterial.Menu {
			id: contextMenu
			QMenuItem { action: view.actionSelectAll }
			QMenuItem { action: view.actionSelectNone }
			Qaterial.MenuSeparator {}
			QMenuItem { action: actionMapAdd }
			QMenuItem { action: actionMapRename }
			QMenuItem { action: actionMapRemove }

		}

		onRightClickOrPressAndHold: {
			if (index != -1)
				currentIndex = index
			contextMenu.popup(mouseX, mouseY)
		}
	}

	QFabButton {
		visible: view.visible
		action: actionMapAdd
	}


	Component {
		id: cmpFile

		QFileDialog {
			title: qsTr("Pálya importálása")
			onFileSelected: {
				handler.mapImport(file)
				Client.Utils.settingsSet("folder/teacherMap", modelFolder.toString())
			}
			folder: Client.Utils.settingsGet("folder/teacherMap", "")
		}

	}

	Action {
		id: actionMapAdd
		text: qsTr("Új pálya")
		icon.source: Qaterial.Icons.plus
		onTriggered: {
			Qaterial.DialogManager.showTextFieldDialog({
														   textTitle: qsTr("Pálya neve"),
														   title: qsTr("Új pálya létrehozása"),
														   standardButtons: Dialog.Cancel | Dialog.Ok,
														   onAccepted: function(_text, _noerror) {
															   /*if (_noerror && _text.length)
																   Client.send(WebSocket.ApiAdmin, "class/create", {
																				   name: _text
																			   })
															   .done(function(r){
																   reload()
															   })
															   .fail(JS.failMessage("Létrehozás sikertelen"))*/
														   }
													   })
		}
	}


	Action {
		id: actionMapRemove
		text: qsTr("Törlés")
		icon.source: Qaterial.Icons.trashCan
		onTriggered: {
			var l = view.getSelected()
			if (!l.length)
				return

			JS.questionDialogPlural(l, qsTr("Biztosan törlöd a kijelölt %1 pályát?"), "name",
									{
										onAccepted: function()
										{
											Client.send(WebSocket.ApiTeacher, "map/delete", {
															list: JS.listGetFields(l, "uuid")
														})
											.done(function(r){
												reload()
											})
											.fail(JS.failMessage("Törlés sikertelen"))
										},
										title: qsTr("Pályák törlése"),
										iconSource: Qaterial.Icons.closeCircle
									})

		}
	}


	Action {
		id: actionMapRename
		text: qsTr("Átnevezés")
		enabled: view.currentIndex != -1
		icon.source: Qaterial.Icons.renameBox
		onTriggered: {
			var o = view.modelGet(view.currentIndex)
			Qaterial.DialogManager.showTextFieldDialog({
														   textTitle: qsTr("Pálya neve"),
														   title: qsTr("Pálya átnevezése"),
														   text: o.name,
														   standardButtons: Dialog.Cancel | Dialog.Ok,
														   onAccepted: function(_text, _noerror) {
															   if (_noerror && _text.length)
																   Client.send(WebSocket.ApiTeacher, "map/%1/update".arg(o.uuid), {
																				   name: _text
																			   })
															   .done(function(r){
																   reload()
															   })
															   .fail(JS.failMessage("Átnevezés sikertelen"))
														   }
													   })
		}
	}



	Action {
		id: actionMapImport
		text: qsTr("Pálya importálása")
		icon.source: Qaterial.Icons.databaseImport
		onTriggered: {
			Qaterial.DialogManager.openFromComponent(cmpFile)
		}
	}


	/*Action {
		id: actionUserAdd
		text: qsTr("Új felhasználó")
		icon.source: Qaterial.Icons.accountPlus
		onTriggered: Client.stackPushPage("AdminUserEdit.qml", {
											  classid: _user.classid
										  })
	}*/


	function reload() {
		view.unselectAll()
		handler.reload()
	}

	StackView.onActivated: reload()

}
