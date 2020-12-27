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

	//title: qsTr("Szerverek")
	//icon: CosStyle.iconUsers

	/*contextMenuFunc: function (m) {
		m.addAction(actionServerNew)
	}*/


	//property alias list: list

	/*SortFilterProxyModel {
		id: userProxyModel
		sourceModel: servers.serversModel
		filters: [
			RegExpFilter {
				enabled: mainSearch.text.length
				roleName: "name"
				pattern: mainSearch.text
				caseSensitivity: Qt.CaseInsensitive
				syntax: RegExpFilter.FixedString
			}
		]
		sorters: [
			StringSorter { roleName: "name" }
		]
		proxyRoles: ExpressionRole {
			name: "details"
			expression: model.host+":"+model.port+(model.username.length ? " - "+model.username : "")
		}
	}*/

	/*QVariantMapProxyView {
		id: list
		anchors.fill: parent

		visible: mapEditor.modelChapterList.count

		model: userProxyModel
		modelTitleRole: "moduleName"
		modelDepthRole: "type"
		modelTitleColorRole: "textColor"
		modelTitleWeightRole: "fontWeight"

		depthWidth: CosStyle.baseHeight*0.5

		autoSelectorChange: false
		autoUnselectorChange: true

		leftComponent: QFontImage {
			width: visible ? list.delegateHeight*0.8 : 0
			height: width
			size: Math.min(height*0.8, 32)

			icon: model && model.module ? mapEditor.moduleData(model.module).icon : ""

			visible: model && model.type === 1

			color: CosStyle.colorPrimary
		}

		rightComponent: QFontImage {
			width: visible ? list.delegateHeight*0.8 : 0
			height: width
			size: Math.min(height*0.8, 32)

			icon: CosStyle.iconClock1

			visible: model && model.storage

			color: CosStyle.colorAccentLighter
		}


		onClicked: {
			var o = list.model.get(index)
			if (o.type === 0) {
				mapEditor.objectiveSelected("")
			} else {
				mapEditor.objectiveSelected(o.uuid)
				mapEditor.run("objectiveLoad", {uuid: o.uuid})
			}
		}

		onRightClicked: contextMenu.popup()

		onLongPressed: {
			if (selectorSet) {
				contextMenu.popup()
				return
			}

			selectorSet = true

			var o = list.model.get(index)

			if (o.type === 0) {
				chaptersFilter.enabled = true
			} else if (o.type === 1) {
				objectiveIdFilter.value = o.id
				objectivesFilter.enabled = true
			}

			mapEditor.modelChapterList.select(serverList.model.mapToSource(serverList.currentIndex))
		}

		onSelectorSetChanged: {
			if (!selectorSet) {
				objectivesFilter.enabled = false
				chaptersFilter.enabled = false
			}
		}


		QMenu {
			id: contextMenu

			MenuItem { action: actionChapterNew }
			MenuItem { action: actionObjectiveNew }
			MenuItem { action: actionRename }
			MenuItem { action: actionRemove }
		}


		onKeyInsertPressed: actionChapterNew.trigger()
		onKeyF4Pressed: actionRename.trigger()
		onKeyDeletePressed: actionRemove.trigger()
		onKeyF2Pressed: actionObjectiveNew.trigger()
	}*/


	/*QPagePanelSearch {
		id: toolbar

		//listView: list

		//labelCountText: servers.serversModel.selectedCount

		//onSelectAll: servers.serversModel.selectAllToggle()

	}


	QToolButtonBig {
		anchors.centerIn: parent
		visible: !serverModel.count
		action: actionServerNew
	}*/



	Action {
		id: actionServerNew
		text: qsTr("Új szerver")
		onTriggered: {
		}
	}




	onPanelActivated: {
	}
}



