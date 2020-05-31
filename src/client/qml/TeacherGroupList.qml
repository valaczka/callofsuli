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

	title: qsTr("Csoportok")
	icon: CosStyle.iconUsers
	maximumWidth: 600

	pageContextMenu: contextMenu

	onPanelActivated: groupList.forceActiveFocus()

	Connections {
		target: pageTeacherGroup
		onPageActivated: groupList.forceActiveFocus()
	}

	Connections {
		target: teacher
		onGroupListLoaded: JS.setModel(baseGroupModel, list)
	}



	QPageHeader {
		id: header

		isSelectorMode: groupList.selectorSet

		labelCountText: groupList.selectedItemCount

		mainItem: QTextField {
			id: mainSearch
			width: parent.width
			lineVisible: false

			placeholderText: qsTr("Keresés...")
			clearAlwaysVisible: true
		}

		onSelectAll: groupList.selectAll()
		onDeselectAll: groupList.selectAll(false)
	}


	ListModel {
		id: baseGroupModel
	}

	SortFilterProxyModel {
		id: userProxyModel
		sourceModel: baseGroupModel
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
	}

	QMenu {
		id: contextMenu

		MenuItem { action: actionCreate }
		MenuItem { action: actionRename }
		MenuItem { action: actionRemove }
	}


	QListItemDelegate {
		id: groupList
		anchors.top: header.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		model: userProxyModel
		isProxyModel: true
		modelTitleRole: "name"

		autoSelectorChange: true

		onClicked: if (!selectorSet) {
					   var m = model.get(index)
					   pageTeacherGroup.groupSelected(m.id, m.name)
				   }

		onRightClicked: contextMenu.popup()

		refreshEnabled: true
		onRefreshRequest: pageTeacherGroup.listReload()

		onKeyDeletePressed: actionRemove.trigger()
		onKeyInsertPressed: actionCreate.trigger()
		onKeyF2Pressed: actionRename.trigger()
	}

	Action {
		id: actionCreate
		text: qsTr("Új csoport")
		icon.source: CosStyle.iconUsersAdd
		onTriggered:  {
			var d = JS.dialogCreateQml("TextField", {title: qsTr("Új csoport neve")})
			d.accepted.connect(function(data) {
				teacher.send({"class": "teacherGroups", "func": "createGroup", "name": data })
			})
			d.open()
		}
	}

	Action {
		id: actionRemove
		enabled: groupList.selectorSet || groupList.currentIndex !== -1
		icon.source: CosStyle.iconRemove
		text: qsTr("Törlés")
		onTriggered: {
			var l = []
			if (groupList.selectorSet)
				l = JS.getSelectedIndices(baseGroupModel, "id")
			else if (groupList.currentIndex != -1)
				l.push(groupList.model.get(groupList.currentIndex).id)

			if (l.length === 0)
				return

			var d = JS.dialogCreateQml("YesNo")
			d.item.title = qsTr("Csoportok törlése")
			d.item.text = qsTr("Biztosan törlöd a kiválasztott %1 csoportot?").arg(l.length)
			d.accepted.connect(function () {
				teacher.send({"class": "teacherGroups", "func": "removeGroup", "list": l})
			})
			d.open()

		}
	}

	Action {
		id: actionRename
		enabled: groupList.currentIndex !== -1
		icon.source: CosStyle.iconRemove
		text: qsTr("Átnevezés")
		onTriggered: {
			var item = groupList.model.get(groupList.currentIndex)
			var d = JS.dialogCreateQml("TextField")
			d.item.title = qsTr("Csoport átnevezése")
			d.item.value = item.name

			d.accepted.connect(function(data) {
				teacher.send({"class": "teacherGroups", "func": "updateGroup", "id": item.id, "name": data })
			})
			d.open()
		}
	}
}
