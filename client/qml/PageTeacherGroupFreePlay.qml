import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPage {
	id: control

	stackPopFunction: function() {
		if (view.selectEnabled) {
			view.unselectAll()
			return false
		}

		return true
	}

	title: group ? group.fullName : qsTr("Csoport")
	subtitle: Client.server ? Client.server.serverName : ""

	property TeacherGroup group: null
	property TeacherMapHandler handler: null

	TeacherGroupFreeMapList {
		id: _mapList
	}

	appBar.backButtonVisible: true

	QListView {
		id: view

		currentIndex: -1
		anchors.fill: parent
		autoSelectChange: true

		refreshProgressVisible: Client.httpConnection.pending
		refreshEnabled: true
		onRefreshRequest: reload()

		model: SortFilterProxyModel {
			sourceModel: _mapList

			sorters: [
				StringSorter {
					roleName: "name"
					sortOrder: Qt.AscendingOrder
					priority: 2
				},
				StringSorter {
					roleName: "missionUuid"
					sortOrder: Qt.AscendingOrder
					priority: 1
				}

			]
		}


		delegate: QItemDelegate {
			property TeacherGroupFreeMap mapObject: model.qtObject
			selectableObject: mapObject

			highlighted: view.selectEnabled ? mapObject && mapObject.selected : ListView.isCurrentItem

			iconSource: Qaterial.Icons.briefcaseCheck
			iconColor: Qaterial.Style.iconColor()

			text: mapObject ? (mapObject.missionUuid !== "" ?
								   mapObject.name + " | " + mapObject.missionName() :
								   mapObject.name)
							: ""

			secondaryText: mapObject && mapObject.map ? qsTr("%1. verzió (%2 @%3)").arg(mapObject.map.version)
														.arg(mapObject.map.lastModified.toLocaleString(Qt.locale(), "yyyy. MMM d. H:mm:ss"))
														.arg(mapObject.map.lastEditor)
													  : ""

		}

		Qaterial.Menu {
			id: _contextMenu
			QMenuItem { action: view.actionSelectAll }
			QMenuItem { action: view.actionSelectNone }
			Qaterial.MenuSeparator {}
			QMenuItem { action: actionMapAdd }
			QMenuItem { action: actionMapRemove }
		}

		onRightClickOrPressAndHold: (index, mouseX, mouseY) => {
										if (index != -1)
										currentIndex = index
										_contextMenu.popup(mouseX, mouseY)
									}

	}

	Qaterial.Banner
	{
		anchors.top: parent.top
		width: parent.width
		drawSeparator: true
		text: qsTr("Ennek a csoportnak nincsen szabadon játszható pályája. Adj hozzá egyet!")
		iconSource: Qaterial.Icons.briefcasePlus
		fillIcon: false
		outlinedIcon: true
		highlightedIcon: true

		action1: qsTr("Hozzáadás")

		onAction1Clicked: actionMapAdd.trigger()

		enabled: group
		visible: group && !_mapList.length
	}

	QFabButton {
		action: actionMapAdd
	}


	ListModel {
		id: _dlgModel

		function open() {
			clear()

			let l=handler.mapList

			for (let i=0; i<l.length; ++i) {
				var m = l.get(i)

				if (!group.findMapInFreePlayMapList(m)) {
					append({
							   uuid: m.uuid,
							   text: m.name,
							   map: m
						   })
				}
			}

			Qaterial.DialogManager.openListView(
						{
							onAccepted: function(idx)
							{
								if (idx === -1)
									return

								_dlgMission.open(_dlgModel.get(idx).map)
							},
							title: qsTr("Pálya hozzáadása"),
							model: _dlgModel
						})
		}
	}


	ListModel {
		id: _dlgMission

		function open(_map) {
			clear()

			let list = group.findMissionsInFreePlayMapList(_map)

			append({
					   uuid: "",
					   text: qsTr("--- Teljes pálya ---")
				   })

			for (let i=0; i<list.length; ++i) {
				append(list[i])
			}

			Qaterial.DialogManager.openListView(
						{
							onAccepted: function(idx)
							{
								if (idx === -1)
									return

								let clist = []

								clist.push({
											   mapuuid: _map.uuid,
											   mission: _dlgMission.get(idx).uuid
										   })

								Client.send(HttpConnection.ApiTeacher, "group/%1/freeplay/add".arg(group.groupid),
											{
												list: clist
											})
								.fail(control, JS.failMessage(qsTr("Hozzáadás sikertelen")))
								.done(control, function(r){
									group.reloadAndCall(control, function() { reloadList()() } )
								})


							},
							title: qsTr("Küldetés kiválasztása"),
							model: _dlgMission
						})
		}
	}

	Action {
		id: actionMapAdd

		text: qsTr("Hozzáadás")
		icon.source: Qaterial.Icons.plus
		enabled: group && handler

		onTriggered: {
			_dlgModel.open()
		}
	}


	Action {
		id: actionMapRemove
		text: qsTr("Eltávolítás")
		icon.source: Qaterial.Icons.minus
		enabled: group && handler
		onTriggered: {
			var l = view.getSelected()
			if (!l.length)
				return

			JS.questionDialogPlural(l, qsTr("Biztosan eltávolítod a kijelölt %1 pályát?"), "name",
									{
										onAccepted: function()
										{
											let list = []

											for (let i=0; i<l.length; ++i) {
												list.push({
															  mapuuid: l[i].map.uuid,
															  mission: l[i].missionUuid
														  })
											}

											Client.send(HttpConnection.ApiTeacher, "group/%1/freeplay/remove".arg(group.groupid),
														{
															list: list
														})
											.done(control, function(r){
												view.unselectAll()
												group.reloadAndCall(control, function() { reloadList()() } )
											})
											.fail(control, JS.failMessage("Eltávolítás sikertelen"))
										},
										title: qsTr("Pályák eltávolítása"),
										iconSource: Qaterial.Icons.briefcaseMinus
									})

		}
	}



	Connections {
		target: handler

		function onReloaded() {
			reloadList()
		}
	}


	Connections {
		target: group

		function onFreePlayMapListChanged() {
			reloadList()
		}
	}



	function selectMap() {

	}

	function selectMission() {
		Qaterial.DialogManager.openRadioListView(
					{
						onAccepted: function(index)
						{
							if (index < 0)
								return

							let ml = _sortedMissionLevelModel.get(index)
							if (ml)
								editor.missionLockAdd(mission, ml.uuid, ml.level)

						},
						title: qsTr("Zárolás hozzáadása"),
						model: _sortedMissionLevelModel
					})
	}


	function reloadList() {
		_mapList.clear()

		if (!group || !handler)
			return

		group.loadToFreePlayMapList(_mapList, handler)
	}

	function reload() {
		view.unselectAll()
		if (handler)
			handler.reload()
	}

	StackView.onActivated: reload()
}
