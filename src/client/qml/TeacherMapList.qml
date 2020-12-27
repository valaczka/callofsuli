import QtQuick 2.12
import QtQuick.Controls 2.12
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	maximumWidth: 600

	title: qsTr("Pályák")
	icon: CosStyle.iconUsers

	property alias list: list

	contextMenuFunc: function (m) {
		m.addAction(actionMapNew)
		m.addAction(actionRename)
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
		proxyRoles: ExpressionRole {
			name: "details"
			expression: "v"+model.version+" "+model.lastModified+" "+cosClient.formattedDataSize(Number(model.dataSize))
		}
	}

	QVariantMapProxyView {
		id: list
		anchors.fill: parent

		visible: teacherMaps.modelMapList.count

		model: userProxyModel
		modelTitleRole: "name"
		modelSubtitleRole: "details"
		/*modelDepthRole: "type"
		modelTitleColorRole: "textColor"
		modelTitleWeightRole: "fontWeight"*/

		autoSelectorChange: true

		/*leftComponent: QFontImage {
			width: visible ? list.delegateHeight*0.8 : 0
			height: width
			size: Math.min(height*0.8, 32)

			icon: model && model.module ? mapEditor.moduleData(model.module).icon : ""

			visible: model && model.type === 1

			color: CosStyle.colorPrimary
		}*/

		rightComponent: QFontImage {
			width: visible ? list.delegateHeight*0.8 : 0
			height: width
			size: Math.min(height*0.8, 32)

			icon: CosStyle.iconClock1

			visible: model && model.editLocked

			color: CosStyle.colorAccentLighter
		}


		onClicked: {
			if (teacherMaps.editUuid.length) {
				cosClient.sendMessageWarning(qsTr("Szerkesztés"), qsTr("Már folyamatban van egy pálya szerkesztése, előbb azt be kell fejezni!"))
			} else {
				var o = list.model.get(index)
				teacherMaps.send("mapEditLock", {uuid: o.uuid})
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
				teacherMaps.send("mapCreate", {name: data})
			})
			d.open()
		}
	}


	Action {
		id: actionRename
		text: qsTr("Átnevezés")
		icon.source: CosStyle.iconAdd
		enabled: !teacherMaps.isBusy && list.currentIndex !== -1
		onTriggered: {
			var o = list.model.get(list.currentIndex)

			var d = JS.dialogCreateQml("TextField", { title: qsTr("Pálya neve"), value: o.name })

			d.accepted.connect(function(data) {

			})
			d.open()
		}
	}

}



