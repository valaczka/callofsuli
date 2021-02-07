import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	maximumWidth: 600
	layoutFillWidth: false

	title: qsTr("Pályák")
	icon: "image://font/AcademicI/\uf15f"

	property alias list: list

	contextMenuFunc: function (m) {
		m.addAction(actionMapNew)
		m.addAction(actionRename)
		m.addAction(actionDownload)
		m.addAction(actionUpload)
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
					ValueFilter {
						roleName: "upload"
						value: true
						SwitchRole.value: CosStyle.colorAccentLighter
					},
					ValueFilter {
						roleName: "download"
						value: true
						SwitchRole.value: CosStyle.colorPrimaryLighter
					}
				]
				defaultValue: CosStyle.colorOKLighter
			},
			SwitchRole {
				name: "fontWeight"
				filters: ExpressionFilter {
					expression: model.local
					SwitchRole.value: Font.Normal
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

			icon: if (model && model.upload)
					  "image://font/AcademicI/\uf114"
				  else if (model && model.download)
					  "image://font/School/\uf137"
				  else
					  "image://font/Academic/\uf118"

			visible: model

			color: model ? model.textColor : CosStyle.colorPrimary
		}

		/*rightComponent: QFontImage {
			width: visible ? list.delegateHeight*0.8 : 0
			height: width
			size: Math.min(height*0.8, 32)

			icon: CosStyle.iconClock1

			visible: model && model.editLocked

			color: CosStyle.colorAccentLighter
		}*/


		onClicked: {
			var o = list.model.get(index)
			if (o.download) {
				list.currentIndex = index
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
			MenuItem { action: actionRename }
			MenuItem { action: actionDownload }
			MenuItem { action: actionUpload }
			MenuItem { action: actionExport }
		}


		onKeyInsertPressed: actionMapNew.trigger()
		onKeyF2Pressed: actionRename.trigger()
		/*onKeyDeletePressed: actionRemove.trigger()
		onKeyF4Pressed: actionObjectiveNew.trigger()*/
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
		id: actionExport
		text: qsTr("Exportálás")
		icon.source: CosStyle.iconDrawer
		enabled: !teacherMaps.isBusy && list.currentIndex !== -1
		onTriggered: {
			var o = list.model.get(list.currentIndex)
			fileDialog.mapUuid = o.uuid
			fileDialog.open()
		}
	}

}



