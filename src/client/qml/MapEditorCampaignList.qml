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

	title: qsTr("Küldetések")
	icon: CosStyle.iconUsers

	contextMenuFunc: function (m) {
		m.addAction(actionCampaignNew)
		m.addAction(actionMissionNew)
	}


	QPageHeader {
		id: header

		visible: mapEditor.campaignModel.count
		isSelectorMode: list.selectorSet

		labelCountText: mapEditor.campaignModel.selectedCount

		mainItem: QTextField {
			id: mainSearch
			width: parent.width

			lineVisible: false
			clearAlwaysVisible: true

			placeholderText: qsTr("Keresés...")
		}

		onSelectAll: mapEditor.campaignModel.selectAllToggle()
	}

	SortFilterProxyModel {
		id: userProxyModel
		sourceModel: mapEditor.campaignModel
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
			StringSorter { roleName: "cname"; priority: 2 },
			RoleSorter { roleName: "cid"; priority: 1 },
			StringSorter { roleName: "mname"; priority: 0 }
		]
		proxyRoles: [
			ExpressionRole {
				name: "name"
				expression: model.type === 0 ? model.cname : model.mname
			},
			SwitchRole {
				name: "textColor"
				filters: ValueFilter {
					roleName: "type"
					value: 0
					SwitchRole.value: CosStyle.colorPrimaryLight
				}
				defaultValue: CosStyle.colorPrimaryLighter
			},
			SwitchRole {
				name: "fontWeight"
				filters: ValueFilter {
					roleName: "type"
					value: 0
					SwitchRole.value: Font.Medium
				}
				defaultValue: Font.Normal
			}
		]

	}



	QVariantMapProxyView {
		id: list
		anchors.top: header.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		visible: mapEditor.campaignModel.count

		model: userProxyModel
		modelTitleRole: "name"
		modelDepthRole: "type"
		modelTitleColorRole: "textColor"
		modelTitleWeightRole: "fontWeight"

		autoSelectorChange: false

		leftComponent: QFontImage {
			width: list.delegateHeight*0.8
			height: width
			size: Math.min(height*0.8, 32)

			icon: CosStyle.iconLock

			visible: model && model.locked

			color: CosStyle.colorPrimary
		}

		/*onClicked: if (servers.editing)
					   actionEdit.trigger()
				   else
					   servers.serverConnect(sourceIndex) */

		onRightClicked: contextMenu.popup()
		onLongPressed: contextMenu.popup()

		QMenu {
			id: contextMenu

			MenuItem { action: actionCampaignNew }
			MenuItem { action: actionCampaignRename }
			MenuItem { action: actionMissionNew }
		}


		onKeyInsertPressed: actionCampaignNew.trigger()
		//onKeyF4Pressed: actionEdit.trigger()
		//onKeyDeletePressed: actionRemove.trigger()
		onKeyF2Pressed: actionMissionNew.trigger()
	}


	QToolButtonBig {
		anchors.centerIn: parent
		visible: !mapEditor.campaignModel.count
		action: actionCampaignNew
	}


	Action {
		id: actionCampaignNew
		text: qsTr("Új hadjárat")
		icon.source: CosStyle.iconAdd
		enabled: !mapEditor.isBusy
		onTriggered: {
			var d = JS.dialogCreateQml("TextField")
			d.item.title = qsTr("Új hadjárat neve")

			d.accepted.connect(function(data) {
				mapEditor.run("campaignAdd", {"name": data})
			})
			d.open()
		}
	}

	Action {
		id: actionCampaignRename
		text: qsTr("Hadjárat átnevezése")
		icon.source: CosStyle.iconAdd
		enabled: !mapEditor.isBusy && list.currentIndex !== -1 && list.model.get(list.currentIndex).type === 0
		onTriggered: {
			var o = list.model.get(list.currentIndex)

			var d = JS.dialogCreateQml("TextField")
			d.item.title = qsTr("Hadjárat neve")
			d.item.value = o.cname

			d.accepted.connect(function(data) {
				mapEditor.run("campaignModify", {
								  "id": o.cid,
								  "data": {"name": data}
							  })
			})
			d.open()
		}
	}


	Action {
		id: actionMissionNew
		text: qsTr("Új küldetés")
		icon.source: CosStyle.iconAdd
		enabled: !mapEditor.isBusy && list.currentIndex !== -1
		onTriggered: {
			var d = JS.dialogCreateQml("TextField")
			d.item.title = qsTr("Új küldetés neve")

			var o = list.model.get(list.currentIndex)

			d.accepted.connect(function(data) {
				mapEditor.run("missionAdd", {"name": data, "campaign": o.cid})
			})
			d.open()
		}
	}

	/*Action {
		id: actionConnect
		text: qsTr("Csatlakozás")
		enabled: list.currentIndex !== -1
		onTriggered: servers.serverConnect(list.sourceIndex)

	}

	Action {
		id: actionEdit
		text: qsTr("Szerkesztés")
		enabled: list.currentIndex !== -1
		onTriggered: {
			servers.serverKey = list.sourceVariantMapModel.getKey(list.sourceIndex)
			servers.editing = true
		}
	}

	Action {
		id: actionRemove
		icon.source: CosStyle.iconRemove
		text: qsTr("Törlés")
		enabled: list.currentIndex !== -1
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
				var si = list.sourceIndex
				var o = list.model.get(list.currentIndex)

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
*/


	Connections {
		target: mapEditor

		function onCampaignAdded(key) {
			console.debug("ADDED", mapEditor.db.get(key, "campaigns", "name"))
		}
	}


	onPanelActivated: {
		list.forceActiveFocus()
	}
}



