import QtQuick 2.12
import QtQuick.Controls 2.12
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property AdminUsers adminUsers: null

	title: qsTr("Osztályok")

	QPageHeader {
		id: header

		isSelectorMode: classList.selectorSet

		labelCountText: classList.selectedItemCount

		searchText.onTextChanged: mainSearch.text = searchText.text

		QTextField {
			id: mainSearch
			width: parent.width

			placeholderText: qsTr("Keresés...")

			onTextChanged: header.searchText.text = mainSearch.text
		}


		rightLoader.sourceComponent: QMenuButton {
			id: userMenu
			anchors.verticalCenter: parent.verticalCenter
			visible: classList.selectorSet

			MenuItem {
				action: actionRemove
			}
		}

		onSelectAll: classList.selectAll()
	}


	ListModel {
		id: baseClassModel
	}

	SortFilterProxyModel {
		id: userProxyModel
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

		model: userProxyModel
		isProxyModel: true
		isObjectModel: true
		modelTitleRole: "name"

		autoSelectorChange: true

		onClicked: pageAdminUsers.classSelected(model.get(index).id)

		onRightClicked: contextMenu.popup()

		QMenu {
			id: contextMenu

			MenuItem { action: actionRename }
			MenuItem { action: actionRemove }
		}
	}

	Action {
		id: actionRemove
		enabled: classList.selectorSet || classList.currentIndex !== -1
		icon.source: CosStyle.iconRemove
		text: qsTr("Törlés")
		onTriggered: deleteClasses()
	}

	Action {
		id: actionRename
		enabled: classList.currentIndex !== -1
		icon.source: CosStyle.iconRemove
		text: qsTr("Átnevezés")
		onTriggered: {
			var item = classList.model.get(classList.currentIndex)
			var d = JS.dialogCreateQml("TextField")
			d.item.title = qsTr("Osztály átnevezése")
			d.item.value = item.name

			d.accepted.connect(function(data) {
				adminUsers.send({"class": "user", "func": "classUpdate", "id": item.id, "name": data })
			})
			d.open()
		}
	}


	Connections {
		target: adminUsers

		onClassListLoaded: getClassList(list)

		onClassCreated: {
			reloadClassList()
			classList.selectAll(false)
		}

		onClassBatchRemoved:  if (data.error)
								  cosClient.sendMessageWarning(qsTr("Osztályok törlése"), qsTr("Szerver hiba"), data.error)
							  else {
								  reloadClassList()
								  classList.selectAll(false)
							  }

		onClassUpdated:  if (data.error)
							 cosClient.sendMessageWarning(qsTr("Osztály átnevezése"), qsTr("Szerver hiba"), data.error)
						 else {
							 reloadClassList()
							 classList.selectAll(false)
						 }
	}

	Connections {
		target: pageAdminUsers

		onCreateClassRequest: {
			var d = JS.dialogCreateQml("TextField")
			d.item.title = qsTr("Új osztály neve")

			d.accepted.connect(function(data) {
				adminUsers.send({"class": "user", "func": "classCreate", "name": data })
			})
			d.open()
		}

	}

	function populated() {
		reloadClassList()
	}

	function reloadClassList() {
		adminUsers.send({"class": "user", "func": "getAllClass"})
	}

	function getClassList(_list) {
		JS.setModel(baseClassModel, _list)
	}

	function deleteClasses() {
		var l = []
		if (classList.selectorSet)
			l = JS.getSelectedIndices(baseClassModel, "id")
		else if (classList.currentIndex != -1)
			l.push(classList.model.get(classList.currentIndex).id)

		if (l.length === 0)
			return

		var d = JS.dialogCreateQml("YesNo")
		d.item.title = qsTr("Osztályok törlése")
		d.item.text = qsTr("Biztosan törlöd a kiválasztott %1 osztályt?").arg(l.length)
		d.accepted.connect(function () {
			adminUsers.send({"class": "user", "func": "classBatchRemove", "list": l})
		})
		d.open()

	}
}
