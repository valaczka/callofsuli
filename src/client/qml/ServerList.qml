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
		m.addAction(actionAutoConnect)
	}


	QPageHeader {
		id: header

		isSelectorMode: serverList.selectorSet

		labelCountText: servers.serversModel.selectedCount

		mainItem: QTextField {
			id: mainSearch
			width: parent.width

			lineVisible: false
			clearAlwaysVisible: true

			placeholderText: qsTr("Keresés...")
		}

		onSelectAll: servers.serversModel.selectAllToggle()
	}

	SortFilterProxyModel {
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
	}



	QVariantMapProxyView {
		id: serverList
		anchors.top: header.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		model: userProxyModel
		modelTitleRole: "name"
		modelSubtitleRole: "details"

		autoSelectorChange: true

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
					   servers.serverConnect(sourceIndex)

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
			servers.serverKey = -1
			servers.editing = true
		}
	}

	Action {
		id: actionConnect
		text: qsTr("Csatlakozás")
		enabled: serverList.currentIndex !== -1
		onTriggered: servers.serverConnect(serverList.sourceIndex)

	}

	Action {
		id: actionEdit
		text: qsTr("Szerkesztés")
		enabled: serverList.currentIndex !== -1
		onTriggered: {
			servers.serverKey = serverList.sourceVariantMapModel.getKey(serverList.sourceIndex)
			servers.editing = true
		}
	}

	Action {
		id: actionRemove
		icon.source: CosStyle.iconRemove
		text: qsTr("Törlés")
		enabled: serverList.currentIndex !== -1
		onTriggered: {
			if (servers.serversModel.selectedCount) {
				var dd = JS.dialogCreateQml("YesNo", {
												title: qsTr("Szerverek törlése"),
												text: qsTr("Biztosan törlöd a kijelölt %1 szervert?").arg(servers.serversModel.selectedCount)
											})
				dd.accepted.connect(function () {
					servers.serverDeleteSelected(servers.serversModel)
					servers.serverKey = -1
				})
				dd.open()
			} else {
				var si = serverList.sourceIndex
				var o = serverList.model.get(serverList.currentIndex)

				var d = JS.dialogCreateQml("YesNo", {
											   title: qsTr("Biztosan törlöd a szervert?"),
											   text: o.name
										   })
				d.accepted.connect(function () {
					servers.serverDelete(si)
					servers.serverKey = -1
				})
				d.open()
			}
		}
	}

	Action {
		id: actionAutoConnect
		text: qsTr("Automata csatlakozás")
		enabled: serverList.currentIndex !== -1
		onTriggered:  {
			servers.serverSetAutoConnect(serverList.sourceIndex)
		}
	}


	onPanelActivated: {
		serverList.forceActiveFocus()
	}
}



