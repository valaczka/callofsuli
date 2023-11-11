import QtQuick 2.15
import QtQuick.Controls 2.15
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

	property TeacherMapHandler handler: null


	QListView {
		id: view

		currentIndex: -1
		anchors.fill: parent
		autoSelectChange: true

		refreshProgressVisible: Client.httpConnection.pending
		refreshEnabled: true
		onRefreshRequest: reload()

		model: SortFilterProxyModel {
			sourceModel: handler ? handler.mapList : null

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
			iconSource: mapObject && mapObject.draftVersion > 0 ? Qaterial.Icons.briefcaseEditOutline : Qaterial.Icons.briefcaseCheck
			iconColor: mapObject && mapObject.draftVersion > 0 ? Qaterial.Colors.green500 : Qaterial.Style.iconColor()
			text: mapObject ? (mapObject.name + (mapObject.draftVersion > 0 ? qsTr(" [*]") : "")) : ""
			secondaryText: mapObject ? qsTr("%1. verzió (%2 @%3)").arg(mapObject.version)
									   .arg(mapObject.lastModified.toLocaleString(Qt.locale(), "yyyy. MMM d. H:mm:ss"))
									   .arg(mapObject.lastEditor)
									   + (mapObject.draftVersion > 0 ? qsTr(" [vázlat]") : "")
									 : ""

			rightSourceComponent: Row {
				QDownloadProgressIcon {
					map: mapObject
					anchors.verticalCenter: parent.verticalCenter
				}
				Qaterial.RoundButton {
					icon.source: Qaterial.Icons.pencil
					icon.color: mapObject && mapObject.draftVersion > 0 ? Qaterial.Colors.green500 : Qaterial.Style.iconColor()
					ToolTip.text: qsTr("Vázlat szerkesztése")
					onClicked: handler.mapEdit(mapObject)
				}
			}

			onClicked: if (mapObject && mapObject.downloaded)
						   //handler.mapEdit(mapObject)
						   console.debug("CLICKED")
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
			Qaterial.MenuSeparator {}
			QMenuItem { action: actionMapPublish }
			QMenuItem { action: actionMapDeleteDraft }
		}

		onRightClickOrPressAndHold: (index, mouseX, mouseY) => {
			if (index != -1)
				currentIndex = index
			contextMenu.popup(mouseX, mouseY)
		}
	}

	Qaterial.Banner
	{
		anchors.top: parent.top
		width: parent.width
		drawSeparator: true
		text: qsTr("Még egyetlen pálya sincsen felvéve. Hozz létre egy újat.")
		iconSource: Qaterial.Icons.briefcasePlusOutline
		fillIcon: false
		outlinedIcon: true
		highlightedIcon: true

		action1: qsTr("Létrehozás")
		action2: qsTr("Importálás")

		onAction1Clicked: actionMapAdd.trigger()
		onAction2Clicked: actionMapImport.trigger()

		visible: handler && !handler.mapList.length
	}

	QFabButton {
		visible: view.visible
		action: actionMapAdd
	}


	Component {
		id: cmpFile

		QFileDialog {
			title: qsTr("Pálya importálása")
			filters: [ "*.map" ]
			onFileSelected: file => {
				handler.mapImport(file)
				Client.Utils.settingsSet("folder/teacherMap", modelFolder.toString())
			}
			folder: Client.Utils.settingsGet("folder/teacherMap", "")
		}

	}

	Action {
		id: actionMapAdd
		text: qsTr("Új pálya")
		icon.source: Qaterial.Icons.briefcasePlus
		enabled: handler
		onTriggered: {
			Qaterial.DialogManager.showTextFieldDialog({
														   textTitle: qsTr("Pálya neve"),
														   title: qsTr("Új pálya létrehozása"),
														   standardButtons: Dialog.Cancel | Dialog.Ok,
														   onAccepted: function(_text, _noerror) {
															   if (_noerror && _text.length)
																   handler.mapCreate(_text)
														   }
													   })
		}
	}


	Action {
		id: actionMapRemove
		text: qsTr("Törlés")
		icon.source: Qaterial.Icons.delete_
		onTriggered: {
			var l = view.getSelected()
			if (!l.length)
				return

			JS.questionDialogPlural(l, qsTr("Biztosan törlöd a kijelölt %1 pályát?"), "name",
									{
										onAccepted: function()
										{
											Client.send(HttpConnection.ApiTeacher, "map/delete", {
															list: JS.listGetFields(l, "uuid")
														})
											.done(control, function(r){
												reload()
											})
											.fail(control, JS.failMessage("Törlés sikertelen"))
										},
										title: qsTr("Pályák törlése"),
										iconSource: Qaterial.Icons.briefcaseRemove
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
																   Client.send(HttpConnection.ApiTeacher, "map/%1/update".arg(o.uuid), {
																				   name: _text
																			   })
															   .done(control, function(r){
																   reload()
															   })
															   .fail(control, JS.failMessage("Átnevezés sikertelen"))
														   }
													   })
		}
	}



	Action {
		id: actionMapImport
		text: qsTr("Pálya importálása")
		icon.source: Qaterial.Icons.briefcaseUploadOutline
		enabled: handler
		onTriggered: {
			if (Qt.platform.os == "wasm") {
				handler.mapImportWasm()
			} else {
				Qaterial.DialogManager.openFromComponent(cmpFile)
			}
		}
	}



	Action {
		id: actionMapPublish
		text: qsTr("Közzététel")
		icon.source: Qaterial.Icons.briefcaseCheck
		onTriggered: {
			var l = view.getSelected()
			if (!l.length)
				return

			var list = []

			for (let i=0; i<l.length; ++i) {
				let o = l[i]
				if (o.draftVersion > 0)
					list.push(o)
			}

			if (!list.length)
				return

			JS.questionDialogPlural(list, qsTr("Biztosan közzéteszed a kijelölt %1 pálya vázlatát?"), "name",
									{
										onAccepted: function()
										{
											for (let j=0; j<list.length; ++j) {
												Client.send(HttpConnection.ApiTeacher, "map/%1/publish/%2".arg(list[j].uuid).arg(list[j].draftVersion))
												.fail(control, JS.failMessage("Közzététel sikertelen"))
											}
											view.unselectAll()
											Client.messageInfo(qsTr("Vázlatok közzétéve"))
										},
										title: qsTr("Vázlatok közzététele"),
										iconSource: Qaterial.Icons.briefcaseCheck
									})

		}
	}

	Action {
		id: actionMapDeleteDraft
		text: qsTr("Vázlat törlése")
		icon.source: Qaterial.Icons.briefcaseRemoveOutline
		onTriggered: {
			var l = view.getSelected()
			if (!l.length)
				return

			var list = []

			for (let i=0; i<l.length; ++i) {
				let o = l[i]
				if (o.draftVersion > 0)
					list.push(o)
			}

			if (!list.length)
				return

			JS.questionDialogPlural(list, qsTr("Biztosan törlöd a kijelölt %1 pálya vázlatát?"), "name",
									{
										onAccepted: function()
										{
											for (let j=0; j<list.length; ++j) {
												Client.send(HttpConnection.ApiTeacher, "map/%1/deleteDraft/%2".arg(list[j].uuid).arg(list[j].draftVersion))
												.fail(control, JS.failMessage("Törlés sikertelen"))
											}
											view.unselectAll()
											Client.messageInfo(qsTr("Vázlatok törölve"))
										},
										title: qsTr("Vázlatok törlése"),
										iconSource: Qaterial.Icons.briefcaseRemoveOutline
									})

		}
	}

	function reload() {
		view.unselectAll()
		if (handler)
			handler.reload()
	}

	StackView.onActivated: reload()

}
