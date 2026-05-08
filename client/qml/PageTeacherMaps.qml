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

		if (_selector.upStack())
			return false

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

	QMapPathSelector {
		id: _selector
		width: view.width
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.top: parent.top
		visible: view.visible
	}

	Qaterial.HorizontalLineSeparator {
		id: _separator
		width: view.width
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.top: _selector.bottom
		visible: view.visible
	}

	QRefreshProgressBar {
		id: progressbar
		anchors.top: parent.top
		visible: Client.httpConnection.pending
	}

	ListModel {
		id: _tagModel
	}

	QListView {
		id: view

		currentIndex: -1

		width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.top: _separator.bottom
		anchors.bottom: parent.bottom

		visible: handler && handler.mapList.length

		clip: true

		autoSelectChange: true

		refreshProgressVisible: false
		refreshEnabled: true
		onRefreshRequest: reload()

		model: SortFilterProxyModel {
			sourceModel: handler ? handler.mapList : null

			filters: [
				ExpressionFilter {
					expression: _selector.currentTagId < 0 ||
								(_selector.currentTagId == 0 && tags.length === 0) ||
								tags.includes(_selector.currentTagId)
				}
			]

			sorters: [
				StringSorter {
					roleName: "name"
					sortOrder: Qt.AscendingOrder
				}
			]
		}

		header: Column {
			Repeater {
				model: SortFilterProxyModel {
					sourceModel: _tagModel

					filters: AnyOf {
						ValueFilter {
							roleName: "parent"
							value: _selector.currentTagId
						}

						ValueFilter {
							roleName: "type"
							value: 1				// Új tag hozzáadása
							enabled: _selector.currentTagId >= 0
						}
					}

					sorters: [
						RoleSorter {
							roleName: "type"
							priority: 2
							sortOrder: Qt.DescendingOrder
						},
						StringSorter {
							roleName: "name"
							priority: 1
						}
					]
				}


				delegate: QIconLoaderItemDelegate {
					id: _tagItem
					width: view.width

					required property int type
					required property int id
					required property string name
					required property int index

					text: name
					iconSource: type == 3 ? Qaterial.Icons.tagMultiple :
											type == 1 ? Qaterial.Icons.tagPlusOutline : Qaterial.Icons.tag
					iconColor: Qaterial.Style.accentColor
					textColor: Qaterial.Style.accentColor


					rightSourceComponent: Qaterial.RoundButton {
						icon.source: Qaterial.Icons.dotsVertical

						visible: _tagItem.type == 2			// Csak normál tag-eknél

						onClicked: _contextMenu.popup()

						Qaterial.Menu {
							id: _contextMenu

							QMenuItem {
								text: qsTr("Átnevezés")
								icon.source: Qaterial.Icons.renameBox

								enabled: _tagItem

								onTriggered: {
									Qaterial.DialogManager.showTextFieldDialog({
																				   textTitle: qsTr("Címke neve"),
																				   title: qsTr("Címke átnevezése"),
																				   text: _tagItem.name,
																				   standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok,
																				   onAccepted: function(_text, _noerror) {
																					   if (_noerror && _text.length && _tagItem)
																						   Client.send(HttpConnection.ApiTeacher, "map/tag/%1/update".arg(_tagItem.id), {
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

							QMenuItem {
								text: qsTr("Törlés")
								icon.source: Qaterial.Icons.tagRemove

								enabled: _tagItem

								onTriggered: {
									JS.questionDialog({
														  onAccepted: function()
														  {
															  Client.send(HttpConnection.ApiTeacher, "map/tag/%1/delete".arg(_tagItem.id))
															  .done(control, function(r){
																  reload()
															  })
															  .fail(control, JS.failMessage("Törlés sikertelen"))
														  },
														  text: qsTr("Biztosan töröld a címkét és minden hozzá tartozó alcímkét?\n%1").arg(_tagItem.name),
														  title: qsTr("Címke törlése"),
														  iconSource: Qaterial.Icons.tagRemove
													  })

								}
							}


						}
					}

					onClicked: {
						if (_tagItem.type == 1) {
							Qaterial.DialogManager.showTextFieldDialog({
																		   textTitle: qsTr("Új címke neve"),
																		   title: qsTr("Új címke létrehozása"),
																		   standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok,
																		   onAccepted: function(_text, _noerror) {
																			   if (_noerror && _text.length)
																				   Client.send(HttpConnection.ApiTeacher, "map/tag/create", {
																								   name: _text,
																								   parent: _selector.currentTagId
																							   })
																			   .done(control, function(r){
																				   reload()
																			   })
																			   .fail(control, JS.failMessage("Címke létrehozása sikertelen"))
																		   }
																	   })
						} else
							_selector.addToStack(_tagItem.id, _tagItem.name)
					}

				}

			}
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
				QTagList {
					visible: mapObject && mapObject.tags.length > 0
					anchors.verticalCenter: parent.verticalCenter
					model: {
						if (!mapObject)
							return []

						let l = []

						for (let i=0; i<mapObject.tags.length; ++i) {
							let id = mapObject.tags[i]
							l.push({
									   color: Qaterial.Style.accentColor,
									   textColor: Qaterial.Colors.black,
									   text: handler.getTagFullName(id)
								   })
						}

						return l
					}
				}

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
			QMenuItem { action: actionMapTag }
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







	function loadModel() {
		_tagModel.clear()

		if (!handler || !handler.tagList)
			return

		if (handler.tagList.count === 0)
			return

		_tagModel.append({
							 type: 3, id: -2, parent: 0, name: qsTr("[Minden pálya]")
						 })

		for (let i=0; i<handler.tagList.count; ++i) {
			let t = handler.tagList.get(i)

			_tagModel.append({
								 type: 2, id: t.tagId, parent: t.parentId, name: t.name
							 })
		}

		_tagModel.append({
							 type: 1, id: -1, parent: -1, name: qsTr("[új címke létrehozása]")
						 })
	}


	onHandlerChanged: {
		loadModel()
		_selector.resetStack()
	}

	Connections {
		target: handler

		function onReloaded() {
			loadModel()
		}
	}

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


	property var _baseMapObjects: []


	Component {
		id: _cmpDialogTag

		QMapTagDialog {
			handler: control.handler
			title: qsTr("Címkék alkalmazása")
			baseMapObject: _baseMapObjects.length > 0 ? _baseMapObjects[0] : null

			onTagSelected: tags => {
							   if (_baseMapObjects.length <= 0) return

							   let u = []

							   for (let i=0; i<_baseMapObjects.length; ++i)
							   u.push(_baseMapObjects[i].uuid)

							   Client.send(HttpConnection.ApiTeacher, "map/tags", {
											   uuids: u,
											   tags: tags
										   })
							   .done(control, function(r){
								   reload()
							   })
							   .fail(control, JS.failMessage("Címkék beállítása sikertelen"))
						   }
		}
	}

	Action {
		id: actionMapTag
		text: qsTr("Címkék")
		icon.source: Qaterial.Icons.tagMultipleOutline
		onTriggered: {
			var l = view.getSelected()
			/*for (let i=0; i<l.length; ++i)
				handler.mapDownload(l[i])
			view.unselectAll()*/

			_baseMapObjects = l

			Qaterial.DialogManager.openFromComponent(_cmpDialogTag)
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
