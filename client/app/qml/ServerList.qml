import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import QtQuick.Controls.Material 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	maximumWidth: 600

	title: qsTr("Szerverek")
	icon: CosStyle.iconUsers

	readonly property bool isDisconnected: cosClient.connectionState == Client.Standby || cosClient.connectionState == Client.Disconnected

	contextMenuFunc: function (m) {
		m.addAction(actionServerSearch)
		m.addAction(actionServerNew)
		m.addAction(actionConnect)
		m.addAction(actionRemove)
		m.addAction(actionEdit)
		m.addAction(actionAutoConnect)
	}


	SortFilterProxyModel {
		id: userProxyModel
		sourceModel: servers.serversModel

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

			frontIcon: CosStyle.iconFavoriteOff
			backIcon: CosStyle.iconFavoriteOn
			color: flipped ? CosStyle.colorAccent : CosStyle.colorPrimaryDark
			flipped: model && model.autoconnect

			mouseArea.onClicked: servers.serverSetAutoConnect(serverList.model.mapToSource(modelIndex))
		}


		onClicked: if (servers.editing)
					   actionEdit.trigger()
				   else
					   servers.serverConnect(serverList.model.mapToSource(index))

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
			servers.serverKey = -1
			servers.editing = true
		}
	}

	Action {
		id: actionConnect
		text: qsTr("Csatlakozás")
		enabled: serverList.currentIndex !== -1
		onTriggered: servers.serverConnect(serverList.model.mapToSource(serverList.currentIndex))

	}

	Action {
		id: actionEdit
		text: qsTr("Szerkesztés")
		enabled: serverList.currentIndex !== -1
		onTriggered: {
			servers.serverKey = serverList.sourceVariantMapModel.getKey(serverList.model.mapToSource(serverList.currentIndex))
			servers.editing = true
		}
	}

	Action {
		id: actionRemove
		icon.source: CosStyle.iconDelete
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
				var si = serverList.model.mapToSource(serverList.currentIndex)
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
			servers.serverSetAutoConnect(serverList.model.mapToSource(serverList.currentIndex))
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



