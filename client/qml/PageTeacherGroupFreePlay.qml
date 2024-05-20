import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import SortFilterProxyModel 0.2
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
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

	TeacherMapList {
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
				}
			]
		}


		delegate: QItemDelegate {
			property TeacherMap mapObject: model.qtObject
			selectableObject: mapObject

			highlighted: view.selectEnabled ? selected : ListView.isCurrentItem

			iconSource: Qaterial.Icons.briefcaseCheck
			iconColor: Qaterial.Style.iconColor()


			text: mapObject ? mapObject.name : ""
			secondaryText: mapObject ? qsTr("%1. verzió (%2 @%3)").arg(mapObject.version)
									   .arg(mapObject.lastModified.toLocaleString(Qt.locale(), "yyyy. MMM d. H:mm:ss"))
									   .arg(mapObject.lastEditor)
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
	}

	Action {
		id: actionMapAdd

		text: qsTr("Hozzáadás")
		icon.source: Qaterial.Icons.plus
		enabled: group && handler

		onTriggered: {
			_dlgModel.clear()

			let l=handler.mapList
			for (let i=0; i<l.length; ++i) {
				var m = l.get(i)
				if (!group.freePlayMapList.includes(m.uuid)) {
					_dlgModel.append({
										 uuid: m.uuid,
										 text: m.name
									 })
				}
			}

			Qaterial.DialogManager.openCheckListView(
						{
							onAccepted: function(list)
							{
								if (!list.length)
									return

								var clist = []

								for (var j=0; j<list.length; j++)
									clist.push(_dlgModel.get(list[j]).uuid)


								Client.send(HttpConnection.ApiTeacher, "group/%1/freeplay/add".arg(group.groupid),
											{ list: clist
											})
								.fail(control, JS.failMessage(qsTr("Hozzáadás sikertelen")))
								.done(control, function(r){
									group.reloadAndCall(control, function() { reloadList()() } )
								})

							},
							title: qsTr("Pályák hozzáadása"),
							model: _dlgModel
						})

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
											Client.send(HttpConnection.ApiTeacher, "group/%1/freeplay/remove".arg(group.groupid),
														{
															list: JS.listGetFields(l, "uuid")
														})
											.done(control, function(r){
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

	function reloadList() {
		_mapList.clear()

		if (!group || !handler)
			return

		let l=handler.mapList

		for (let i=0; i<l.length; ++i) {
			var m = l.get(i)

			if (group.freePlayMapList.includes(m.uuid)) {
				_mapList.append(m)
			}
		}
	}

	function reload() {
		view.unselectAll()
		if (handler)
			handler.reload()
	}

	StackView.onActivated: reload()
}
