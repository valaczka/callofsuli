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

	title: qsTr("Csoportok")
	icon: "image://font/AcademicI/\uf15f"

	property alias list: list

	contextMenuFunc: function (m) {
		m.addAction(actionGroupNew)
		m.addAction(actionRename)
	}

	SortFilterProxyModel {
		id: userProxyModel
		sourceModel: teacherGroups.modelGroupList
		sorters: [
			StringSorter { roleName: "name" }
		]
	}

	QVariantMapProxyView {
		id: list
		anchors.fill: parent

		visible: teacherGroups.modelGroupList.count

		model: userProxyModel
		modelTitleRole: "name"

		autoSelectorChange: true

		delegateHeight: CosStyle.twoLineHeight

		leftComponent: QFontImage {
			width: visible ? list.delegateHeight : 0
			height: width*0.8
			size: Math.min(height*0.8, 32)

			icon: "image://font/Academic/\uf118"

			visible: model

			color: CosStyle.colorPrimary
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
			/*if (o.download) {
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
			}*/
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

			MenuItem { action: actionGroupNew }
			MenuItem { action: actionRename }
		}


		onKeyInsertPressed: actionGroupNew.trigger()
		onKeyF2Pressed: actionRename.trigger()
		/*onKeyDeletePressed: actionRemove.trigger()
		onKeyF4Pressed: actionObjectiveNew.trigger()*/
	}


	QToolButtonBig {
		anchors.centerIn: parent
		visible: !teacherGroups.modelGroupList.count
		action: actionGroupNew
	}



	Action {
		id: actionGroupNew
		text: qsTr("Új csoport")
		icon.source: CosStyle.iconAdd
		enabled: !teacherGroups.isBusy
		onTriggered: {
			var d = JS.dialogCreateQml("TextField")
			d.item.title = qsTr("Új csoport neve")

			d.accepted.connect(function(data) {
				if (data.length)
					teacherGroups.send("groupCreate", {name: data})
			})
			d.open()
		}
	}


	Action {
		id: actionRename
		text: qsTr("Átnevezés")
		icon.source: CosStyle.iconRename
		enabled: !teacherGroups.isBusy && list.currentIndex !== -1
		onTriggered: {
			var o = list.model.get(list.currentIndex)

			var d = JS.dialogCreateQml("TextField", { title: qsTr("Csoport neve"), value: o.name })

			d.accepted.connect(function(data) {
				if (data.length)
					teacherGroups.send("groupUpdate", {id: o.id, name: data})
			})
			d.open()
		}
	}



	/*Action {
		id: actionExport
		text: qsTr("Exportálás")
		icon.source: CosStyle.iconDrawer
		enabled: !teacherMaps.isBusy && list.currentIndex !== -1
		onTriggered: {
			var o = list.model.get(list.currentIndex)
			fileDialog.mapUuid = o.uuid
			fileDialog.open()
		}
	}*/

}


