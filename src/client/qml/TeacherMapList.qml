import QtQuick 2.12
import QtQuick.Controls 2.12
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property Teacher teacher: null

	title: qsTr("Pályák")
	icon: CosStyle.iconUsers
	maximumWidth: 600

	pageContextMenu: contextMenu

	onPanelActivated: mapList.forceActiveFocus()

	Connections {
		target: pageTeacherMap
		onPageActivated: mapList.forceActiveFocus()
	}

	Connections {
		target: teacher
		onMapListLoaded: JS.setModel(baseMapModel, list)
	}



	QPageHeader {
		id: header

		isSelectorMode: mapList.selectorSet

		labelCountText: mapList.selectedItemCount

		mainItem: QTextField {
			id: mainSearch
			width: parent.width
			lineVisible: false

			placeholderText: qsTr("Keresés...")
			clearAlwaysVisible: true
		}

		onSelectAll: mapList.selectAll()
		onDeselectAll: mapList.selectAll(false)
	}


	ListModel {
		id: baseMapModel
	}

	SortFilterProxyModel {
		id: userProxyModel
		sourceModel: baseMapModel
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

		proxyRoles: [
			/*ExpressionRole {
				name: "details"
				expression: "v"+model.version+" "+qsTr("módosítva:")+" "+model.timeModified
			},*/
			SwitchRole {
				name: "textColor"
				filters: ValueFilter {
					roleName: "hasGroup"
					value: false
					SwitchRole.value: "#88000000"
				}
				defaultValue: "black"
			}
		]
	}

	QMenu {
		id: contextMenu

		MenuItem { action: actionCreate }
		MenuItem { action: actionClone }
		MenuItem { action: actionRename }
		MenuItem { action: actionRemove }
		MenuSeparator { }
		MenuItem { action: actionImport }
	}


	QListItemDelegate {
		id: mapList
		anchors.top: header.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		model: userProxyModel
		isProxyModel: true
		modelTitleRole: "name"
		//modelSubtitleRole: "details"
		modelTitleColorRole: "textColor"
		modelSubtitleColorRole: "textColor"

		autoSelectorChange: true

		//delegateHeight: CosStyle.twoLineHeight

		onClicked: if (!selectorSet) {
					   var m = model.get(index)
					   pageTeacherMap.mapSelected(m.id, m.name)
				   }

		onRightClicked: contextMenu.popup()

		refreshEnabled: true
		onRefreshRequest: pageTeacherMap.listReload()

		onKeyDeletePressed: actionRemove.trigger()
		onKeyInsertPressed: actionCreate.trigger()
		onKeyF2Pressed: actionRename.trigger()
	}

	Action {
		id: actionCreate
		text: qsTr("Új pálya")
		icon.source: CosStyle.iconUsersAdd
		onTriggered:  {
			var d = JS.dialogCreateQml("TextField", {title: qsTr("Új pálya neve")})
			d.accepted.connect(function(data) {
				teacher.send({"class": "teacherMaps", "func": "createMap", "name": data })
			})
			d.open()
		}
	}

	Action {
		id: actionClone
		text: qsTr("Duplikálás")
		icon.source: CosStyle.iconAdd
		//onTriggered: teacher.send({"class": "teacherGroups", "func": "getExcludedMaps", "id": groupId})
	}

	Action {
		id: actionImport
		text: qsTr("Importálás")
		icon.source: CosStyle.iconAdd
		//onTriggered: teacher.send({"class": "teacherGroups", "func": "getExcludedMaps", "id": groupId})
	}

	Action {
		id: actionRemove
		enabled: mapList.selectorSet || mapList.currentIndex !== -1
		icon.source: CosStyle.iconRemove
		text: qsTr("Törlés")
		onTriggered: {
			var l = []
			if (mapList.selectorSet)
				l = JS.getSelectedIndices(baseMapModel, "id")
			else if (mapList.currentIndex != -1)
				l.push(mapList.model.get(mapList.currentIndex).id)

			if (l.length === 0)
				return

			var d = JS.dialogCreateQml("YesNo")
			d.item.title = qsTr("Pályák törlése")
			d.item.text = qsTr("Biztosan törlöd a kiválasztott %1 pályát?").arg(l.length)
			d.accepted.connect(function () {
				teacher.send({"class": "teacherMaps", "func": "removeMap", "list": l})
			})
			d.open()

		}
	}

	Action {
		id: actionRename
		enabled: mapList.currentIndex !== -1
		icon.source: CosStyle.iconRemove
		text: qsTr("Átnevezés")
		onTriggered: {
			var item = mapList.model.get(mapList.currentIndex)
			var d = JS.dialogCreateQml("TextField")
			d.item.title = qsTr("Pálya átnevezése")
			d.item.value = item.name

			d.accepted.connect(function(data) {
				teacher.send({"class": "teacherMaps", "func": "updateMap", "id": item.id, "name": data })
			})
			d.open()
		}
	}
}
