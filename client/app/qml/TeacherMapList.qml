import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
	id: control

	title: qsTr("Pályák")
	icon: CosStyle.iconPlanet

	signal selectMap()

	QObjectListView {
		id: list
		anchors.fill: parent

		refreshEnabled: true
		delegateHeight: CosStyle.twoLineHeight

		autoSelectorChange: true

		header: QTabHeader {
			tabContainer: control
			isPlaceholder: true
		}

		leftComponent: QFontImage {
			width: visible ? list.delegateHeight : 0
			height: width*0.8
			size: Math.min(height*0.8, 32)

			icon: if (model && model.downloaded)
					  "image://font/Academic/\uf118"
				  else
					  "image://font/School/\uf137"

			visible: model

			color: model && model.downloaded ? model.textColor : CosStyle.colorWarning
		}

		model: SortFilterProxyModel {
			sourceModel: teacherMaps.modelMapList

			sorters: [
				StringSorter { roleName: "name" }
			]

			proxyRoles: [
				ExpressionRole {
					name: "details"
					expression: qsTr("%1. verzió (%2), módosítva: %3") .arg(model.version) .arg(cosClient.formattedDataSize(Number(model.dataSize))) .arg(model.lastModified.toLocaleString(Qt.locale()))
				},
				SwitchRole {
					name: "textColor"
					filters: [
						ExpressionFilter {
							expression: model.used > 0
							SwitchRole.value: CosStyle.colorAccent
						},
						ExpressionFilter {
							expression: model.binded.length
							SwitchRole.value: CosStyle.colorPrimaryLighter
						}
					]
					defaultValue: CosStyle.colorPrimary
				}
			]
		}

		modelTitleRole: "name"
		modelSubtitleRole: "details"
		modelTitleColorRole: "textColor"
		modelSubtitleColorRole: "textColor"

		highlightCurrentItem: false

		onRefreshRequest: teacherMaps.send("mapListGet", { })

		footer: Column {
			QToolButtonFooter {
				width: list.width
				action: actionUpload
				text: qsTr("Új pálya feltöltése")
			}

			QToolButtonFooter {
				width: list.width
				action: actionMapEditor
				color: CosStyle.colorAccent
			}
		}

		onClicked: {
			teacherMaps.selectedMapId = modelObject(index).uuid
			selectMap()
		}

		onRightClicked: contextMenu.popup()

		onLongPressed: {
			if (selectorSet) {
				contextMenu.popup()
				return
			}
		}



		QMenu {
			id: contextMenu

			MenuItem { action: actionDownload }
		}
	}

	QIconEmpty {
		visible: teacherMaps.modelMapList.count === 0
		anchors.centerIn: parent
		textWidth: parent.width*0.75
		text: qsTr("Egyetlen pálya sincs még feltöltve")
		tabContainer: control
	}

	Action {
		id: actionDownload
		text: qsTr("Letöltés")
		icon.source: CosStyle.iconDownload
		enabled: !teacherMaps.isBusy && (list.currentIndex !== -1 || teacherMaps.modelMapList.selectedCount)
		onTriggered: {
			var o = list.modelObject(list.currentIndex)

			var more = teacherMaps.modelMapList.selectedCount

			if (more > 0)
				teacherMaps.mapDownload({ list: teacherMaps.modelMapList.getSelectedData("uuid") })
			else
				teacherMaps.mapDownload({ uuid: o.uuid })
		}
	}

	Action {
		id: actionUpload
		text: qsTr("Feltöltés")
		icon.source: CosStyle.iconUpload
		onTriggered: {
			var d = JS.dialogCreateQml("File", {
										   isSave: false,
										   folder: cosClient.getSetting("mapFolder", ""),
										   title: qsTr("Feltöltés")
									   })
			d.accepted.connect(function(data){
				teacherMaps.mapUpload(data)
				cosClient.setSetting("mapFolder", d.item.modelFolder)
			})

			d.open()
		}
	}

}



