import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPagePanel {
	id: panel

	property string objectiveUuid: ""

	layoutFillWidth: true

	title: objectiveUuid.length ? qsTr("Célpont") : ""
	icon: "image://font/Academic/\uf1ca"


	contextMenuFunc: function (m) {
		m.addAction(actionRemove)
	}


	Connections {
		target: mapEditor


		function onObjectiveSelected(uuid) {
			objectiveUuid = uuid
		}


		function onObjectiveLoaded(data) {
			if (!Object.keys(data).length) {
				objectiveUuid = ""
				return
			}

			if (data.uuid !== objectiveUuid)
				return

			moduleLoader.setSource("MapEditorObjectiveEdit_"+data.objectiveModule+".qml", {
									   objectiveUuid: objectiveUuid,
									   moduleData: data
								   })

		}


		function onObjectiveRemoved() {
			reloadData()
		}
	}

	Connections {
		target: mapEditor.db

		function onUndone() {
			reloadData()
		}
	}


	QLabel {
		id: noLabel
		opacity: !objectiveUuid.length
		visible: opacity != 0

		anchors.centerIn: parent

		text: qsTr("Válassz célpontot")

		Behavior on opacity { NumberAnimation { duration: 125 } }
	}



	Loader {
		id: moduleLoader

		anchors.fill: parent

		opacity: (objectiveUuid.length && status != Loader.Null) ? 1.0 : 0.0
		visible: opacity != 0

		Behavior on opacity { NumberAnimation { duration: 125 } }

		onVisibleChanged: if (!visible)
							  source = ""

		onLoaded: item.reloadData()
	}



	Action {
		id: actionRemove
		text: qsTr("Törlés")
		icon.source: CosStyle.iconDelete
		enabled: !mapEditor.isBusy && objectiveUuid.length
		onTriggered: {
			mapEditor.run("objectiveRemove", {"uuid": objectiveUuid})
		}
	}



	function reloadData() {
		mapEditor.run("objectiveLoad", {uuid: objectiveUuid})
	}


}
