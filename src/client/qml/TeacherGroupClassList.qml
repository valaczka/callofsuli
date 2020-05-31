import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.14
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property Teacher teacher: null
	property int groupId: -1

	title: qsTr("Osztályok")
	icon: CosStyle.iconUsers

	pageContextMenu: QMenu {
		MenuItem { action: actionAdd }
		MenuItem { action: actionRemove }
	}

	QPageHeader {
		id: header

		anchors.top: parent.top
		width: parent.width

		isSelectorMode: classList.selectorSet
		labelCountText: classList.selectedItemCount

		mainItem: RowLayout {
			id: row1
			spacing: 0

			width: parent.width

			QTextField {
				id: mainSearch
				lineVisible: false

				Layout.fillWidth: true
				Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

				placeholderText: qsTr("Keresés...")
				clearAlwaysVisible: true
			}

			QToolButton {
				action: actionAdd
				display: Button.IconOnly

				Layout.fillWidth: false
				Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
			}

			QToolButton {
				action: actionRemove
				display: Button.IconOnly
				visible: classList.selectorSet || classList.currentIndex != -1

				Layout.fillWidth: false
				Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
			}

		}

		onSelectAll: classList.selectAll()
		onDeselectAll: classList.selectAll(false)
	}

	ListModel {
		id: baseClassModel
	}

	SortFilterProxyModel {
		id: classProxyModel
		sourceModel: baseClassModel
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


	QListItemDelegate {
		id: classList
		anchors.top: header.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		model: classProxyModel
		isProxyModel: true
		modelTitleRole: "name"

		autoSelectorChange: true

		refreshEnabled: true
		onRefreshRequest: pageGroupEdit.getGroupData()

		onKeyDeletePressed: actionRemove.trigger()
		onKeyInsertPressed: actionAdd.trigger()
	}



	Action {
		id: actionAdd
		text: qsTr("Hozzáadás")
		icon.source: CosStyle.iconAdd
		onTriggered: teacher.send({"class": "teacherGroups", "func": "getExcludedClasses", "id": groupId})
	}

	Action {
		id: actionRemove
		text: qsTr("Eltávolítás")
		icon.source: CosStyle.iconRemove
		enabled: classList.selectorSet || classList.currentIndex != -1
		onTriggered: {
			var s = []

			if (classList.selectorSet)
				s = JS.getSelectedIndices(baseClassModel, "id")
			else if (classList.currentIndex != -1){
				var id = classList.model.get(classList.currentIndex).id
				s.push(id)
			}


			var l = []

			for (var i=0; i<baseClassModel.count; ++i) {
				var o = baseClassModel.get(i)
				if (!s.includes(o.id))
					l.push(o.id)
			}

			var d = JS.dialogCreateQml("YesNo")
			d.item.title = qsTr("Osztályok eltávolítása")
			d.item.text = qsTr("Biztosan eltávolítod a kiválasztott %1 osztályt?").arg(s.length)
			d.accepted.connect(function () {
				teacher.send({"class": "teacherGroups", "func": "updateGroup", "id": groupId, "classes": l})
				classList.selectAll(false)
			})
			d.open()
		}
	}

	Connections {
		target: teacher

		onGroupReceived: JS.setModel(baseClassModel, groupData.classes)

		onGroupExcludedClassesReceived: loadDialogClasses(groupData.list)
	}

	onPanelActivated: classList.forceActiveFocus()


	function loadDialogClasses(list) {
		var d = JS.dialogCreateQml("List")
		d.item.title = qsTr("Osztályok")
		d.item.list.selectorSet = true
		d.item.list.modelTitleRole = "name"
		d.item.list.modelSelectedRole = "selected"

		JS.setModel(d.item.model, list)

		d.accepted.connect(function() {
			var maps = JS.getSelectedIndices(d.item.model, "id")
			teacher.send({"class": "teacherGroups", "func": "addClasses", "id": groupId, "list": maps})
		})
		d.open()
	}
}
