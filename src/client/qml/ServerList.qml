import QtQuick 2.12
import QtQuick.Controls 2.12
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property Servers servers: null

	maximumWidth: 600


	title: qsTr("Szerverek")
	icon: CosStyle.iconUsers


	pageContextMenu: swipeMode ? menuSwipe : menuNoSwipe

	QMenu {
		id: menuSwipe
		MenuItem { action: actionServerNew }
		MenuItem { action: actionConnect }
		MenuItem { action: actionRemove }
		MenuItem { action: actionEdit }

		MenuSeparator { }

		MenuItem { action: actionAbout }
		MenuItem { action: actionExit }
	}

	QMenu {
		id: menuNoSwipe
		MenuItem { action: actionServerNew }
		MenuItem { action: actionConnect }
		MenuItem { action: actionRemove }
		MenuItem { action: actionEdit }
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

		onClicked: pageStart.serverConnect(model.get(index).id)

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



		/*Keys.onPressed: {
			if (event.key === Qt.Key_Insert) {
				editServer(-1)
			} else if (event.key === Qt.Key_F4 && listMenu.currentIndex !== -1) {
				editServer(listMenu.model[listMenu.currentIndex].id)
			} else if (event.key === Qt.Key_Delete && listMenu.currentIndex !== -1) {
				deleteServer(listMenu.currentIndex)
			} else if (event.key === Qt.Key_F1) {
				var o = JS.createPage("MapEditor", {}, page)
				o.pagePopulated.connect(function() {
					o.map.loadFromFile("AAA.cosm")
					o.map.mapOriginalFile = "AAA.cosm"
					o.mapName = "AAA.cosm"
				})
			}
		}*/


	}



	Action {
		id: actionServerNew
		text: qsTr("Új szerver")
		onTriggered: pageStart.serverCreate()
	}

	Action {
		id: actionConnect
		text: qsTr("Csatlakozás")
		enabled: serverList.currentIndex !== -1
		onTriggered: pageStart.serverConnect(serverList.model.get(serverList.currentIndex).id)

	}

	Action {
		id: actionEdit
		text: qsTr("Szerkesztés")
		enabled: serverList.currentIndex !== -1
		onTriggered: pageStart.serverEdit(serverList.model.get(serverList.currentIndex).id)
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
		onTriggered:  servers.serverSetAutoConnect(serverList.model.get(serverList.currentIndex).id)
	}


	Connections {
		target: servers

		onServerListLoaded: JS.setModel(baseServerModel, serverList)
		onServerInfoUpdated: servers.serverListReload()
	}

	onPopulated: {
		servers.serverListReload()
		serverList.forceActiveFocus()
	}

	onPanelActivated: serverList.forceActiveFocus()
}



