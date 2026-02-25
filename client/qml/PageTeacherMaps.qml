import QtQuick
import QtQuick.Controls
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
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
			Qaterial.MenuSeparator {}
			QMenuItem { action: actionMapDownload }
			QMenuItem { action: actionMapExport }
		}
	}

	property TeacherMapHandler handler: null


	QListView {
		id: view

		currentIndex: -1

		height: parent.height
		width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
		anchors.horizontalCenter: parent.horizontalCenter

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
									   .arg(JS.readableTimestamp(mapObject.lastModified))
									   .arg(mapObject.lastEditor)
									   + (mapObject.draftVersion > 0 ? qsTr(" [vázlat]") : "")
									 : ""

			rightSourceComponent: Row {
				QDownloadProgressIcon {
					map: mapObject
					anchors.verticalCenter: parent.verticalCenter
				}
				Qaterial.RoundButton {
					icon.source: Qaterial.Icons.briefcaseArrowLeftRightOutline
					icon.color: /*mapObject && mapObject.draftVersion > 0 ? Qaterial.Colors.green500 :*/ Qaterial.Style.iconColor()
					ToolTip.text: qsTr("Vázlat feltöltése (csere)")
					onClicked: {
						_importToMap = mapObject
						Qaterial.DialogManager.openFromComponent(cmpFile)
					}
				}
			}

			onClicked: if (mapObject && mapObject.downloaded)
						   handler.mapEdit(mapObject)
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
			Qaterial.MenuSeparator {}
			QMenuItem { action: actionMapDownload }
			QMenuItem { action: actionMapExport }
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

	/*QFabButton {
		visible: view.visible
		action: actionMapAdd
	}*/


	property TeacherMap _importToMap: null

	Component {
		id: cmpFile

		QFileDialog {
			title: _importToMap ? qsTr("Vázlat importálás") : qsTr("Pálya importálása")
			filters: [ "*.map" ]
			onFileSelected: file => {
								if (_importToMap)
									handler.mapReplace(_importToMap, file)
								else
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
														   standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok,
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
														   standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok,
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
		id: actionMapDownload
		text: qsTr("Letöltés")
		icon.source: Qaterial.Icons.download
		onTriggered: {
			var l = view.getSelected()
			for (let i=0; i<l.length; ++i)
				handler.mapDownload(l[i])
			view.unselectAll()
		}
	}


	Action {
		id: actionMapExport
		text: qsTr("Exportálás")
		icon.source: Qaterial.Icons.export_
		onTriggered: {
			var l = view.getSelected()
			for (let i=0; i<l.length; ++i) {
				if (!l[i].downloaded) {
					Client.messageWarning(qsTr("%1 pály nincs letöltve").arg(l[i].name), qsTr("Exportálás"))
					return
				}
			}

			/*if (Qt.platform.os == "wasm")
				mapEditor.wasmSaveAs(false)
			else*/
				Qaterial.DialogManager.openFromComponent(_cmpFileExport)
		}
	}


	Component {
		id: _cmpFileExport

		QFileDialog {
			title: qsTr("Pályák exportálás")
			filters: [ "*.tar" ]
			isSave: true
			suffix: ".tar"
			onFileSelected: file => {
				if (Client.Utils.fileExists(file))
					overrideQuestion(file)
				else
					exportToFile(file)
				Client.Utils.settingsSet("folder/mapEditor", modelFolder.toString())
			}

			folder: Client.Utils.settingsGet("folder/mapEditor")
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
				_importToMap = null
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
											handler.reload()
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
											handler.reload()
											Client.messageInfo(qsTr("Vázlatok törölve"))
										},
										title: qsTr("Vázlatok törlése"),
										iconSource: Qaterial.Icons.briefcaseRemoveOutline
									})

		}
	}



	function overrideQuestion(file) {
		JS.questionDialog({
							  onAccepted: function()
							  {
								  exportToFile(file)
							  },
							  text: qsTr("A fájl létezik. Felülírjuk?\n%1").arg(file),
							  title: qsTr("Mentés másként"),
							  iconSource: Qaterial.Icons.fileAlert
						  })
	}



	function exportToFile(file) {
		var l = view.getSelected()
		if (!l.length)
			return

		if (handler.mapExport(file, l)) {
			Client.messageInfo(qsTr("Az exportálás sikerült: %1").arg(file.toString()))
			view.unselectAll()
		} else {
			Client.messageWarning(qsTr("Az exportálás sikertelen"), qsTr("Hiba"))
		}
	}


	function reload() {
		view.unselectAll()
		if (handler)
			handler.reload()
	}

	StackView.onActivated: reload()

}
