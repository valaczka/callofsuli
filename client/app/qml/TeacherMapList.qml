import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QSwipeContainer {
	id: panel

	title: qsTr("Pályák")
	icon: CosStyle.iconBooks

	property alias buttonUpload: buttonUpload
	property alias list: list

	QVariantMapProxyView {
		id: list
		anchors.fill: parent

		visible: teacherMaps.modelMapList.count

		refreshEnabled: true
		delegateHeight: CosStyle.twoLineHeight

		autoSelectorChange: true


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
					expression: qsTr("%1. verzió (%2), módosítva: %3") .arg(model.version) .arg(cosClient.formattedDataSize(Number(model.dataSize))) .arg(model.lastModified)
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

		footer: QToolButtonFooter {
			width: list.width
			action: buttonUpload.action
		}

		onClicked: teacherMaps.selectedMapId = model.get(index).uuid

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

	QToolButtonBig {
		id: buttonUpload
		anchors.centerIn: parent
		visible: !list.visible
		text: qsTr("Új pálya feltöltése")
		color: CosStyle.colorAccent
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
				teacherMaps.mapDownload({ list: teacherMaps.modelMapList.getSelectedData("uuid") })
			else
				teacherMaps.mapDownload({ uuid: o.uuid })
		}
	}

}



