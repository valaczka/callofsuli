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

	title: qsTr("Pályák")
	icon: CosStyle.iconUsers

	pageContextMenu: QMenu {
		MenuItem { action: actionAdd }
		MenuItem { action: actionRemove }
	}

	QPageHeader {
		id: header

		anchors.top: parent.top
		width: parent.width

		isSelectorMode: mapList.selectorSet
		labelCountText: mapList.selectedItemCount

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
				visible: mapList.selectorSet || mapList.currentIndex != -1

				Layout.fillWidth: false
				Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
			}

		}

		onSelectAll: mapList.selectAll()
		onDeselectAll: mapList.selectAll(false)
	}

	ListModel {
		id: baseMapModel
	}

	SortFilterProxyModel {
		id: mapProxyModel
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
	}


	QListItemDelegate {
		id: mapList
		anchors.top: header.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		model: mapProxyModel
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
		onTriggered: teacher.send({"class": "teacherGroups", "func": "getExcludedMaps", "id": groupId})
	}

	Action {
		id: actionRemove
		text: qsTr("Eltávolítás")
		icon.source: CosStyle.iconRemove
		enabled: mapList.selectorSet || mapList.currentIndex != -1
		onTriggered: {
			var s = []

			if (mapList.selectorSet)
				s = JS.getSelectedIndices(baseMapModel, "id")
			else if (mapList.currentIndex != -1){
				var id = mapList.model.get(mapList.currentIndex).id
				s.push(id)
			}

			var l = []

			for (var i=0; i<baseMapModel.count; ++i) {
				var o = baseMapModel.get(i)
				if (!s.includes(o.id))
					l.push(o.id)
			}

			var d = JS.dialogCreateQml("YesNo")
			d.item.title = qsTr("Pályák eltávolítása")
			d.item.text = qsTr("Biztosan eltávolítod a kiválasztott %1 pályát?").arg(s.length)
			d.accepted.connect(function () {
				teacher.send({"class": "teacherGroups", "func": "updateGroup", "id": groupId, "maps": l})
				mapList.selectAll(false)
			})
			d.open()
		}
	}



	Connections {
		target: teacher

		onGroupReceived: JS.setModel(baseMapModel, groupData.maps)

		onGroupExcludedMapsReceived: loadDialogMaps(groupData.list)
	}

	onPanelActivated: mapList.forceActiveFocus()


	function loadDialogMaps(list) {
		var d = JS.dialogCreateQml("List")
		d.item.title = qsTr("Pályák")
		d.item.list.selectorSet = true
		d.item.list.modelTitleRole = "name"
		d.item.list.modelSelectedRole = "selected"

		JS.setModel(d.item.model, list)

		d.accepted.connect(function() {
			var maps = JS.getSelectedIndices(d.item.model, "id")
			teacher.send({"class": "teacherGroups", "func": "addMaps", "id": groupId, "list": maps})
		})
		d.open()
	}
}
