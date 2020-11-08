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

	title: qsTr("Szerverek")
	icon: CosStyle.iconUsers

	contextMenuFunc: function (m) {
		m.addAction(actionServerNew)
		m.addAction(actionConnect)
		m.addAction(actionRemove)
		m.addAction(actionEdit)
	}


	QPageHeader {
		id: header

		isSelectorMode: serverList.selectorSet

		labelCountText: serverList.selectedItemCount

		mainItem: QTextField {
			id: mainSearch
			width: parent.width

			lineVisible: false
			clearAlwaysVisible: true

			placeholderText: qsTr("Keresés...")
		}

		onSelectAll: serverList.selectAll()
		onDeselectAll: serverList.selectAll(false)
	}


	ListModel {
		id: baseServerModel
	}

	SortFilterProxyModel {
		id: userProxyModel
		sourceModel: baseServerModel
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
		id: serverList
		anchors.top: header.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		model: userProxyModel
		isProxyModel: true
		modelTitleRole: "name"

		autoSelectorChange: false


		leftComponent: QFontImage {
			width: serverList.delegateHeight
			height: width
			size: Math.min(height*0.8, 32)

			icon: model && model.autoconnect ? CosStyle.iconFavoriteOn : CosStyle.iconFavoriteOff

			color: CosStyle.colorAccent
		}

		onClicked: if (servers.editing)
					   actionEdit.trigger()
				   else
					   servers.serverConnect(model.get(index).id)

		onRightClicked: contextMenu.popup()
		onLongPressed: contextMenu.popup()

		QMenu {
			id: contextMenu

			MenuItem { action: actionConnect }
			MenuItem { action: actionEdit}
			MenuItem { action: actionRemove }
			MenuSeparator {}
			MenuItem { action: actionAutoConnect }
		}


		onKeyInsertPressed: actionServerNew.trigger()
		onKeyF4Pressed: actionEdit.trigger()
		onKeyDeletePressed: actionRemove.trigger()
		onKeyF2Pressed: actionAutoConnect.trigger()
	}



	Action {
		id: actionServerNew
		text: qsTr("Új szerver")
		onTriggered: {
			servers.serverId = -1
			servers.editing = true
		}
	}

	Action {
		id: actionConnect
		text: qsTr("Csatlakozás")
		enabled: serverList.currentIndex !== -1
		onTriggered: servers.serverConnect(serverList.model.get(serverList.currentIndex).id)

	}

	Action {
		id: actionEdit
		text: qsTr("Szerkesztés")
		enabled: serverList.currentIndex !== -1
		onTriggered: {
			servers.serverId = serverList.model.get(serverList.currentIndex).id
			servers.editing = true
		}
	}

	Action {
		id: actionRemove
		icon.source: CosStyle.iconRemove
		text: qsTr("Törlés")
		enabled: serverList.currentIndex !== -1
		onTriggered: {
			var o = serverList.model.get(serverList.currentIndex)

			var d = JS.dialogCreateQml("YesNo", {
										   title: qsTr("Biztosan törlöd a szervert?"),
										   text: o.name
									   })
			d.accepted.connect(function () {
				servers.serverInfoDelete(o.id)
			})
			d.open()
		}
	}

	Action {
		id: actionAutoConnect
		text: qsTr("Automata csatlakozás")
		enabled: serverList.currentIndex !== -1
		onTriggered:  {
			var d = serverList.model.get(serverList.currentIndex)
			servers.serverSetAutoConnect(d.id, !d.autoconnect)
		}
	}


	Connections {
		target: servers

		onServerListLoaded: JS.setModel(baseServerModel, serverList)
		onServerInfoUpdated: servers.serverListReload()
	}

	onPopulated: {
		servers.serverListReload()
	}

	onPanelActivated: {
		serverList.forceActiveFocus()
	}
}



