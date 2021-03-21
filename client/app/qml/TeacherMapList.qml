import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	layoutFillWidth: true

	title: qsTr("Pályák")
	icon: "image://font/AcademicI/\uf15f"

	property alias list: list

	contextMenuFunc: function (m) {
		m.addAction(actionMapNew)
		m.addAction(actionImport)
		m.addSeparator()
		m.addAction(actionRename)
		m.addAction(actionDownload)
		m.addAction(actionRemove)
		m.addSeparator()
		m.addAction(actionEdit)
		m.addAction(actionUpload)
		m.addAction(actionLocalRemove)
	}

	SortFilterProxyModel {
		id: userProxyModel
		sourceModel: teacherMaps.modelMapList
		filters: [
			RegExpFilter {
				enabled: toolbar.searchBar.text.length
				roleName: "name"
				pattern: toolbar.searchBar.text
				caseSensitivity: Qt.CaseInsensitive
				syntax: RegExpFilter.FixedString
			}
		]
		sorters: [
			StringSorter { roleName: "name" }
		]
		proxyRoles: [
			ExpressionRole {
				name: "details"
				expression: (model.localLastModified !== undefined ? model.localLastModified+" " : "")
							+"("+cosClient.formattedDataSize(Number(model.dataSize))+")"
			},
			SwitchRole {
				name: "textColor"
				filters: [
					AllOf {
						ValueFilter {
							roleName: "used"
							value: true
						}
						ValueFilter {
							roleName: "upload"
							value: true
						}
						SwitchRole.value: CosStyle.colorErrorLighter
					},
					ValueFilter {
						roleName: "used"
						value: true
						SwitchRole.value: CosStyle.colorOKLighter
					},
					ValueFilter {
						roleName: "download"
						value: true
						SwitchRole.value: CosStyle.colorPrimaryDarker
					},
					ValueFilter {
						roleName: "local"
						value: true
						SwitchRole.value: CosStyle.colorAccent
					},
					ValueFilter {
						roleName: "upload"
						value: true
						SwitchRole.value: CosStyle.colorAccentLighter
					}
				]
				defaultValue: CosStyle.colorPrimaryLighter
			},
			SwitchRole {
				name: "fontWeight"
				filters: ExpressionFilter {
					expression: model.local
					SwitchRole.value: Font.DemiBold
				}
				defaultValue: Font.Medium
			}

		]
	}

	QVariantMapProxyView {
		id: list
		anchors.fill: parent

		visible: teacherMaps.modelMapList.count

		model: userProxyModel
		modelTitleRole: "name"
		modelSubtitleRole: "details"
		modelTitleColorRole: "textColor"
		modelSubtitleColorRole: "textColor"
		modelTitleWeightRole: "fontWeight"

		autoSelectorChange: true

		delegateHeight: CosStyle.twoLineHeight

		leftComponent: QFontImage {
			width: visible ? list.delegateHeight : 0
			height: width*0.8
			size: Math.min(height*0.8, 32)

			icon: if (model && model.local)
					  CosStyle.iconAdd
				  else if (model && model.upload)
					  "image://font/AcademicI/\uf114"
				  else if (model && model.download)
					  "image://font/School/\uf137"
				  else
					  "image://font/Academic/\uf118"

			visible: model

			color: model ? model.textColor : CosStyle.colorPrimary
		}


		rightComponent: Row {
			visible: !teacherMaps.modelMapList.selectedCount
			spacing: 0


			QToolButton {
				anchors.verticalCenter: parent.verticalCenter
				ToolTip.text: qsTr("Letöltés")

				icon.source: CosStyle.iconDownload
				visible: model && model.download
				onClicked: {
					list.currentIndex = modelIndex
					actionDownload.trigger()
				}
			}

			QToolButton {
				anchors.verticalCenter: parent.verticalCenter
				ToolTip.text: qsTr("Szerkesztés")

				icon.source: CosStyle.iconEdit
				visible: model && model.upload
				onClicked: {
					list.currentIndex = modelIndex
					actionEdit.trigger()
				}
			}
		}

		onClicked: {
			var o = list.model.get(index)
			if (o.local) {
				teacherMaps.mapSelect("")
			} else {
				teacherMaps.mapSelect(o.uuid)
			}

		}

		onRightClicked: contextMenu.popup()

		onLongPressed: {
			if (selectorSet) {
				contextMenu.popup()
				return
			}

			/*selectorSet = true

			var o = list.model.get(index)

			if (o.type === 0) {
				chaptersFilter.enabled = true
			} else if (o.type === 1) {
				objectiveIdFilter.value = o.id
				objectivesFilter.enabled = true
			}*/

			//mapEditor.modelChapterList.select(serverList.model.mapToSource(serverList.currentIndex))
		}



		QMenu {
			id: contextMenu

			MenuItem { action: actionMapNew }
			MenuSeparator { }
			MenuItem { action: actionRename }
			MenuItem { action: actionDownload }
			MenuItem { action: actionRemove }
			MenuSeparator { }
			MenuItem { action: actionEdit }
			MenuItem { action: actionUpload }
			MenuItem { action: actionExport }
			MenuItem { action: actionLocalRemove }
		}


		onKeyInsertPressed: actionMapNew.trigger()
		onKeyF2Pressed: actionRename.trigger()
		onKeyDeletePressed: actionRemove.trigger()
		onKeyF4Pressed: actionEdit.trigger()
	}


	QPagePanelSearch {
		id: toolbar

		listView: list
		labelCountText: teacherMaps.modelMapList.selectedCount
		onSelectAll: teacherMaps.modelMapList.selectAllToggle()

	}


	QToolButtonBig {
		anchors.centerIn: parent
		visible: !teacherMaps.modelMapList.count
		action: actionMapNew
		color: CosStyle.colorOK
	}



	Action {
		id: actionMapNew
		text: qsTr("Új pálya")
		icon.source: CosStyle.iconAdd
		enabled: !teacherMaps.isBusy
		onTriggered: {
			var d = JS.dialogCreateQml("TextField")
			d.item.title = qsTr("Új pálya neve")

			d.accepted.connect(function(data) {
				if (data.length)
					teacherMaps.mapAdd({name: data})
			})
			d.open()
		}
	}


	Action {
		id: actionRename
		text: qsTr("Átnevezés")
		icon.source: CosStyle.iconRename
		enabled: !teacherMaps.isBusy && list.currentIndex !== -1
		onTriggered: {
			var o = list.model.get(list.currentIndex)

			var d = JS.dialogCreateQml("TextField", { title: qsTr("Pálya neve"), value: o.name })

			d.accepted.connect(function(data) {
				if (data.length)
					teacherMaps.mapRename({uuid: o.uuid, name: data, local: o.local})
			})
			d.open()
		}
	}


	Action {
		id: actionDownload
		text: qsTr("Letöltés")
		icon.source: CosStyle.iconDownload
		enabled: !teacherMaps.isBusy && (list.currentIndex !== -1 || teacherMaps.modelMapList.selectedCount)
		onTriggered: {
			var o = list.model.get(list.currentIndex)

			var more = teacherMaps.modelMapList.selectedCount

			if (more > 0)
				teacherMaps.mapDownload({list: teacherMaps.modelMapList.getSelectedData("uuid") })
			else
				teacherMaps.mapDownload({uuid: o.uuid})
		}
	}

	Action {
		id: actionUpload
		text: qsTr("Feltöltés")
		icon.source: CosStyle.iconUploadCloud
		enabled: !teacherMaps.isBusy && (list.currentIndex !== -1 || teacherMaps.modelMapList.selectedCount)
		onTriggered: {
			var o = list.model.get(list.currentIndex)

			var more = teacherMaps.modelMapList.selectedCount

			if (more > 0)
				teacherMaps.mapUpload({list: teacherMaps.modelMapList.getSelectedData("uuid") })
			else
				teacherMaps.mapUpload({uuid: o.uuid})
		}
	}


	Action {
		id: actionEdit
		text: qsTr("Szerkesztés")
		icon.source: CosStyle.iconEdit
		enabled: !teacherMaps.isBusy && list.currentIndex !== -1
		onTriggered: {
			var o = list.model.get(list.currentIndex)

			if (o.download) {
				actionDownload.trigger()
			} else if (o.upload) {
				JS.createPage("MapEditor", {
								  database: teacherMaps.db,
								  databaseTable: "localmaps",
								  databaseUuid: o.uuid
							  })
			} else {
				var d = JS.dialogCreateQml("YesNo", {
											   title: qsTr("Szerkesztés"),
											   text: qsTr("Készítsünk egy helyi másolatát a szerkesztéshez?\n%1").arg(o.name)
										   })
				d.accepted.connect(function() {
					teacherMaps.mapLocalCopy({uuid: o.uuid})
				})

				d.open()
			}
		}
	}

	Action {
		id: actionExport
		text: qsTr("Exportálás")
		icon.source: CosStyle.iconDrawer
		enabled: Qt.platform.os === "linux" && !teacherMaps.isBusy && list.currentIndex !== -1
		onTriggered: {
			var o = list.model.get(list.currentIndex)
			fileDialog.mapUuid = o.uuid
			fileDialog.open()
		}
	}


	Action {
		id: actionImport
		text: qsTr("Importálás")
		icon.source: CosStyle.iconDrawer
		enabled: !teacherMaps.isBusy
		onTriggered: {
			teacherMaps.mapImport()
		}
	}



	Action {
		id: actionRemove
		icon.source: CosStyle.iconDeleteCloud
		text: qsTr("Törlés")
		enabled: !teacherMaps.isBusy && (list.currentIndex !== -1 || teacherMaps.modelMapList.selectedCount)
		onTriggered: {
			if (teacherMaps.modelMapList.selectedCount) {
				var dd = JS.dialogCreateQml("YesNo", {
												title: qsTr("Pályák törlése a szerveren"),
												text: qsTr("Biztosan törlöd a szerveren a kijelölt %1 pályát?")
												.arg(teacherMaps.modelMapList.selectedCount)
											})
				dd.accepted.connect(function () {
					teacherMaps.send("mapRemove", {"list": teacherMaps.modelMapList.getSelectedData("uuid") })
				})
				dd.open()
			} else {
				var o = list.model.get(list.currentIndex)

				var d = JS.dialogCreateQml("YesNo", {
											   title: qsTr("Biztosan törlöd a pályát a szerveren?"),
											   text: o.name
										   })
				d.accepted.connect(function () {
					teacherMaps.send("mapRemove", {"uuid": o.uuid })
				})
				d.open()
			}
		}
	}

	Action {
		id: actionLocalRemove
		icon.source: CosStyle.iconDelete
		text: qsTr("Helyi törlés")
		enabled: !teacherMaps.isBusy && list.currentIndex !== -1 && list.model.get(list.currentIndex).upload
		onTriggered: {
				var o = list.model.get(list.currentIndex)

				var d = JS.dialogCreateQml("YesNo", {
											   title: qsTr("Biztosan törlöd a pálya helyi példányát?"),
											   text: o.name
										   })
				d.accepted.connect(function () {
					teacherMaps.mapLocalRemove({uuid: o.uuid})
				})
				d.open()
		}
	}


	onPopulated: list.forceActiveFocus()
}



