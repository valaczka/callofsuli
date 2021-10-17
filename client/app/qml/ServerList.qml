import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import QtQuick.Controls.Material 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QSimpleContainer {
	id: panel

	maximumWidth: 600

	title: qsTr("Szerverek")
	icon: CosStyle.iconComputer

	readonly property bool isDisconnected: cosClient.connectionState == Client.Standby || cosClient.connectionState == Client.Disconnected

	property var contextMenuFunc: function (m) {
		m.addAction(actionServerNew)
		m.addAction(actionServerSearch)
	}


	SortFilterProxyModel {
		id: userProxyModel
		sourceModel: servers.serversModel

		sorters: [
			StringSorter { roleName: "name" }
		]
		proxyRoles: ExpressionRole {
			name: "details"
			expression: (model.ssl ? "wss://" : "ws://") + model.host+":"+model.port+(model.username.length ? " - "+model.username : "")
						+(model.broadcast ? qsTr(" (auto)") : "")
		}
	}



	QVariantMapProxyView {
		id: serverList
		anchors.fill: parent

		visible: servers.serversModel.count && isDisconnected

		model: userProxyModel
		modelTitleRole: "name"
		modelSubtitleRole: "details"

		autoSelectorChange: true

		delegateHeight: CosStyle.twoLineHeight

		leftComponent: QFlipable {
			id: flipable
			width: serverList.delegateHeight
			height: width

			frontIcon: CosStyle.iconUnchecked
			backIcon: CosStyle.iconSelected
			color: flipped ? CosStyle.colorAccent : CosStyle.colorPrimaryDark
			flipped: model && model.autoconnect

			mouseArea.onClicked: servers.serverSetAutoConnect(model.id)
		}

		/*footer: QToolButtonFooter {
			width: serverList.width
			height: Math.max(implicitHeight, serverList.delegateHeight)
			text: qsTr("Pályaszerkesztő")
			icon.source: "image://font/Academic/\uf118"
			onClicked: {
				JS.createPage("MapEditor", {})
			}
		}*/

		footer: QToolButtonFooter {
			width: serverList.width
			action: actionServerNew
		}



		onClicked: servers.serverConnect(serverList.model.get(index).id)

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



	QToolButtonBig {
		anchors.centerIn: parent
		visible: !servers.serversModel.count
		action: actionServerSearch
		color: CosStyle.colorOK
	}



	Column {
		anchors.centerIn: parent
		visible: !isDisconnected

		spacing: 10

		Row {
			spacing: 10
			anchors.horizontalCenter: parent.horizontalCenter


			BusyIndicator {
				anchors.verticalCenter: parent.verticalCenter
				height: CosStyle.pixelSize*3
				width: CosStyle.pixelSize*3
				running: true
				Material.accent: CosStyle.colorPrimaryLighter
			}

			QLabel {
				anchors.verticalCenter: parent.verticalCenter
				text: qsTr("Kapcsolódás...")
				font.pixelSize: CosStyle.pixelSize*1.2
				color: CosStyle.colorPrimary
			}

		}

		QButton {
			anchors.horizontalCenter: parent.horizontalCenter
			themeColors: CosStyle.buttonThemeRed
			text: qsTr("Mégsem")
			icon.source: CosStyle.iconCancel
			onClicked: cosClient.closeConnection()
		}
	}



	Action {
		id: actionServerNew
		text: qsTr("Hozzáadás")
		icon.source: CosStyle.iconAdd
		onTriggered: {
			servers.uiAdd()
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
			var key = servers.serversModel.findKey("id", serverList.model.get(serverList.currentIndex).id)
			servers.uiEdit(key)
		}
	}

	Action {
		id: actionRemove
		icon.source: CosStyle.iconDelete
		text: qsTr("Törlés")
		enabled: serverList.currentIndex !== -1
		onTriggered: {
			var more = servers.serversModel.selectedCount

			if (more > 0) {
				var dd = JS.dialogCreateQml("YesNo", {
												title: qsTr("Szerverek törlése"),
												text: qsTr("Biztosan törlöd a kijelölt %1 szervert?").arg(more)
											})
				dd.accepted.connect(function () {
					servers.serverDelete({list: servers.serversModel.getSelectedData("id")})
					servers.serversModel.unselectAll()
				})
				dd.open()
			} else {
				var o = serverList.model.get(serverList.currentIndex)

				var d = JS.dialogCreateQml("YesNo", {
											   title: qsTr("Szerver törlése"),
											   text: qsTr("Biztosan törlöd a szervert?\n%1").arg(o.name)
										   })
				d.accepted.connect(function () {
					servers.serverDelete({id: o.id})
					servers.serversModel.unselectAll()
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
			servers.serverSetAutoConnect(serverList.model.get(serverList.currentIndex).id)
		}
	}


	Action {
		id: actionServerSearch
		text: qsTr("Keresés")
		icon.source: CosStyle.iconSearch
		onTriggered: {
			servers.sendBroadcast()
		}
	}


	onPopulated: serverList.forceActiveFocus()
}



