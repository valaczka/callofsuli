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
	icon: CosStyle.iconUsers

	property alias list: list

	contextMenuFunc: function (m) {
		m.addAction(actionDownload)
	}


	SortFilterProxyModel {
		id: userProxyModel
		sourceModel: studentMaps.modelMapList
		sorters: [
			StringSorter { roleName: "name" }
		]
	}


	QVariantMapProxyView {
		id: list
		anchors.fill: parent

		visible: studentMaps.modelMapList.count

		model: userProxyModel
		modelTitleRole: "name"

		autoSelectorChange: true


		refreshEnabled: true

		//delegateHeight: CosStyle.twoLineHeight

		leftComponent: QFontImage {
			width: visible ? list.delegateHeight : 0
			height: width*0.8
			size: Math.min(height*0.8, 32)

			icon: if (model && model.downloaded)
					  CosStyle.iconOK
				  else
					  CosStyle.iconDown

			visible: model

			color: model && model.downloaded ? CosStyle.colorPrimary : CosStyle.colorAccent
		}

		/*rightComponent: QFontImage {
			width: visible ? list.delegateHeight*0.8 : 0
			height: width
			size: Math.min(height*0.8, 32)

			icon: CosStyle.iconClock1

			visible: model && model.editLocked

			color: CosStyle.colorAccentLighter
		}*/


		onRefreshRequest: studentMaps.send("mapListGet")

		onClicked: {
			var o = list.model.get(index)
			if (o.downloaded) {
				studentMaps.mapLoad({uuid: o.uuid, name: o.name})
			} else {
				list.currentIndex = index
				actionDownload.trigger()
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

			MenuItem { action: actionDownload }
		}


		//onKeyInsertPressed: actionMapNew.trigger()
		//onKeyF2Pressed: actionRename.trigger()
		/*onKeyDeletePressed: actionRemove.trigger()
		onKeyF4Pressed: actionObjectiveNew.trigger()*/
	}






	Action {
		id: actionDownload
		text: qsTr("Letöltés")
		icon.source: CosStyle.iconDown
		enabled: !studentMaps.isBusy && (list.currentIndex !== -1 || studentMaps.modelMapList.selectedCount)
		onTriggered: {
			var o = list.model.get(list.currentIndex)

			var more = studentMaps.modelMapList.selectedCount

			if (more > 0)
				studentMaps.mapDownload({list: studentMaps.modelMapList.getSelectedData("uuid") })
			else
				studentMaps.mapDownload({uuid: o.uuid})
		}
	}



}



