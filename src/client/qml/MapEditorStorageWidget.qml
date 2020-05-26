import QtQuick 2.12
import QtQuick.Controls 2.14
import COS.Client 1.0
import QtQuick.Layouts 1.14
import "."
import "Style"
import "JScript.js" as JS

QListItemDelegate {
	id: list

	property MapEditor map: null
	property int missionId: -1
	property int summaryId: -1

	signal storageSelected(int id)
	signal objectiveSelected(int id)

	isObjectModel: true

	delegateHeight: CosStyle.twoLineHeight

	modelTitleRole: "name"
	modelSubtitleRole: "modulename"
	modelDepthRole: "depth"


	rightComponent: QLabel {
		color: "black"
		text: model && model.rightText ? model.rightText : ""
	}

	onClicked: {
		var o = list.model.get(index)
		if (o.sid >= 0 && o.oid === -1)
			storageSelected(o.sid)
		else if (o.sid === -1 && o.oid >= 0) {
			objectiveSelected(o.oid)
		} else if (o.sid >= 0 && o.oid === -2) {
			loadDialogObjectives(o.sid, o.module)
		} else if (o.sid === -2) {
			loadDialogStorages()
		}
	}


	function load(storages) {
		list.model.clear()

		for (var i=0; i<storages.length; ++i) {
			var s = storages[i]

			var t = map.storageModule(s.module)

			list.model.append({sid: s.id, oid: -1, name: s.name, modulename: (t ? t.label : qsTr("???")), module: s.module, depth: 0})

			for (var j=0; j<s.objectives.length; ++j) {
				var o = s.objectives[j]
				var tt = map.objectiveModule(o.module)

				var rightText = o.level

				list.model.append({sid: -1, oid: o.id, name: (tt ? tt.label : qsTr("???")), modulename: "", module: o.module, depth: 1, rightText: rightText})
			}

			list.model.append({sid: s.id, oid: -2, name: qsTr("-- fegyver hozzáadása --"), modulename: "", module: s.module, depth: 1})
		}

		list.model.append({sid: -2, oid: -1, name: qsTr("-- célpont hozzáadása --"), modulename: "", module: "", depth: 0})
	}



	function loadDialogStorages() {
		var d = JS.dialogCreateQml("List")
		d.item.title = qsTr("Célpontok")
		d.item.newField.visible = false
		d.item.simpleSelect = true
		d.item.list.modelTitleRole = "label"

		JS.setModel(d.item.model, map.storageModules)

		d.accepted.connect(function(idx) {
			var s = map.storageModules[idx]
			map.undoLogBegin(qsTr("Célpont hozzáadása"))
			map.storageAdd({name: s.label, module: s.type, data: s.defaultData ? s.defaultData : "{}"}, missionId, summaryId)
			map.undoLogEnd()
		})
		d.open()
	}


	function loadDialogObjectives(sId, sType) {
		var d = JS.dialogCreateQml("List")
		d.item.title = qsTr("Fegyverek")
		d.item.newField.visible = false
		d.item.simpleSelect = true
		d.item.list.modelTitleRole = "label"

		JS.setModel(d.item.model, map.storageObjectiveModules(sType))

		d.accepted.connect(function(idx) {
			var s = map.objectiveModule(d.item.model.get(idx).type)
			map.undoLogBegin(qsTr("Fegyver hozzáadása"))
			map.objectiveAdd({storageid: sId, module: s.type, data: s.defaultData ? s.defaultData : "{}" }, missionId, summaryId)
			map.undoLogEnd()
		})
		d.open()
	}
}
