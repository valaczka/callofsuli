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

	property alias campaignsModel: userProxyModel.sourceModel

	SortFilterProxyModel {
		id: userProxyModel

		sourceModel: mapEditor.newModel([
											"type",
											"cid",
											"uuid",
											"cname",
											"mname",
											"mandatory",
											"locked"
										])
		filters: [
			AllOf {
				RegExpFilter {
					enabled: toolbar.searchBar.text.length
					roleName: "name"
					pattern: toolbar.searchBar.text
					caseSensitivity: Qt.CaseInsensitive
					syntax: RegExpFilter.FixedString
				}
				ValueFilter {
					id: campaignsFilter
					enabled: false
					roleName: "type"
					value: 0
				}
				AllOf {
					id: missionsFilter
					enabled: false
					ValueFilter {
						roleName: "type"
						value: 1
					}
					ValueFilter {
						id: missionCidFilter
						roleName: "cid"
						value: -1
					}
				}
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
		anchors.fill: parent

		visible: campaignsModel.count

		model: userProxyModel
		modelTitleRole: "name"
		modelDepthRole: "type"
		modelTitleColorRole: "textColor"
		modelTitleWeightRole: "fontWeight"

		depthWidth: CosStyle.baseHeight*0.5

		autoSelectorChange: false

		leftComponent: QFlipable {
			id: flipable
			width: visible ? list.delegateHeight : 0
			height: width

			visible: model && model.type === 1

			frontIcon: CosStyle.iconFavoriteOff
			backIcon: CosStyle.iconFavoriteOn
			color: flipped ? CosStyle.colorAccent : CosStyle.colorPrimaryDark
			flipped: model && model.mandatory

			mouseArea.onClicked: mapEditor.run("missionModify", {
												   "uuid": model.uuid,
												   "data": {"mandatory": !model.mandatory}
											   })
		}

		rightComponent: QFontImage {
			width: visible ? list.delegateHeight*0.8 : 0
			height: width
			size: Math.min(height*0.8, 32)

			icon: CosStyle.iconLock

			visible: model && model.locked

			color: CosStyle.colorPrimary
		}


		onClicked: {
			var o = list.model.get(list.currentIndex)
			if (o.type === 0)
				mapEditor.run("campaignLoad", {id: o.cid})
			else
				mapEditor.run("missionLoad", {uuid: o.uuid})
		}

		onRightClicked: contextMenu.popup()

		onLongPressed: {
			if (selectorSet)
				return

			selectorSet = true

			var o = list.model.get(index)

			if (o.type === 0) {
				campaignsFilter.enabled = true
			} else if (o.type === 1) {
				missionCidFilter.value = o.cid
				missionsFilter.enabled = true
			}

			campaignsModel.select(sourceIndex)
		}

		onSelectorSetChanged: {
			if (!selectorSet) {
				missionsFilter.enabled = false
				campaignsFilter.enabled = false
			}
		}


		QMenu {
			id: contextMenu

			MenuItem { action: actionCampaignNew }
			MenuItem { action: actionMissionNew }
			MenuItem { action: actionRename }
			MenuItem { action: actionRemove }
		}


		onKeyInsertPressed: actionCampaignNew.trigger()
		onKeyF4Pressed: actionRename.trigger()
		onKeyDeletePressed: actionRemove.trigger()
		onKeyF2Pressed: actionMissionNew.trigger()
	}


	QPagePanelSearch {
		id: toolbar

		listView: list

		enabled: campaignsModel.count
		labelCountText: campaignsModel.selectedCount
		onSelectAll: campaignsModel.selectAllToggle()
	}


	QToolButtonBig {
		anchors.centerIn: parent
		visible: !campaignsModel.count
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
		id: actionRename
		text: qsTr("Átnevezés")
		icon.source: CosStyle.iconAdd
		enabled: !mapEditor.isBusy && list.currentIndex !== -1
		onTriggered: {
			var o = list.model.get(list.currentIndex)

			var d = JS.dialogCreateQml("TextField")
			if (o.type === 0) {
				d.item.title = qsTr("Hadjárat neve")
				d.item.value = o.cname
			} else {
				d.item.title = qsTr("Küldetés neve")
				d.item.value = o.mname
			}

			d.accepted.connect(function(data) {
				if (o.type === 0)
					mapEditor.run("campaignModify", {
									  "id": o.cid,
									  "data": {"name": data}
								  })
				else
					mapEditor.run("missionModify", {
									  "uuid": o.uuid,
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

	Action {
		id: actionRemove
		text: qsTr("Törlés")
		icon.source: CosStyle.iconDelete
		enabled: !mapEditor.isBusy && list.currentIndex !== -1
		onTriggered: {
			var o = list.model.get(list.currentIndex)

			var d = JS.dialogCreateQml("YesNo")
			d.item.title = o.name
			d.item.text = o.type === 0 ? qsTr("Biztosan törlöd a hadjáratot a küldetéseivel együtt?")
									   : qsTr("Biztosan törlöd a küldetést?")

			d.accepted.connect(function(data) {
				if (o.type === 0)
					mapEditor.run("campaignRemove", {"id": o.cid})
				else
					mapEditor.run("missionRemove", {"uuid": o.uuid})
			})
			d.open()
		}
	}


	Connections {
		target: mapEditor

		function onCampaignListReloaded(list) {
			campaignsModel.setVariantList(list, "uuid");
		}
	}


	Component.onCompleted: {
		mapEditor.run("campaignListReload")
	}

	onPanelActivated: {
		list.forceActiveFocus()
	}
}



