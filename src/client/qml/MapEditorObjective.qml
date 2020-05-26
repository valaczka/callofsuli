import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panelObjective

	property MapEditor map: null
	property int storageId: -1
	property int objectiveId: -1
	property int parentMissionId: -1
	property int parentSummaryId: -1

	title: objectiveId === -1 ? qsTr("Célpont") : qsTr("Fegyver")

	QLabel {
		id: noLabel
		opacity: storageId === -1 && objectiveId === -1
		visible: opacity != 0

		text: qsTr("Válassz célpontot vagy fegyvert")

		Behavior on opacity { NumberAnimation { duration: 125 } }
	}

	Loader {
		id: editorLoader
		anchors.fill: parent
		opacity: (storageId !== -1 || objectiveId !== -1) && editorLoader.status === Loader.Ready
		visible: opacity != 0
	}


	Connections {
		target: pageEditor
		onStorageSelected: {
			storageId = id
			objectiveId = -1
			parentMissionId = parentMId
			parentSummaryId = parentSId
			get()
		}

		onObjectiveSelected: {
			objectiveId = id
			storageId = -1
			parentMissionId = parentMId
			parentSummaryId = parentSId
			get()
		}
	}


	Connections {
		target: map
		onUndone: get()
	}


	function populated() { get() }


	function get() {
		var d = {}
		if (storageId !== -1) {
			d = map.storageGet(storageId)
			var i = map.storageInfo(d.module)
			if (Object.keys(i).length) {
				var qml = "MOD"+i.type+"Editor.qml"
				if (editorLoader.source === qml)
					editorLoader.item.editorData = d
				else
					editorLoader.setSource(qml, { editorData: d })
			} else {
				editorLoader.source = ""
			}
		} else if (objectiveId !== -1) {
			d = map.objectiveGet(objectiveId)

			i = map.objectiveInfo(d.module)
			if (Object.keys(i).length) {
				qml = "MOD"+i.type+"Editor.qml"
				if (editorLoader.source === qml) {
					editorLoader.item.editorData = d
				} else
					editorLoader.setSource(qml, { editorData: d })
			} else {
				editorLoader.source = ""
			}
		} else {
			editorLoader.source = ""
		}
	}
}
