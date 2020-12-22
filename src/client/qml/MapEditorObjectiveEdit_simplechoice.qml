import QtQuick 2.12
import QtQuick.Controls 2.12
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QAccordion {
	id: control

	anchors.fill: parent

	property string objectiveUuid: ""
	property var moduleData: null

	MapEditorObjectiveEditHeader {
		id: header
		title: qsTr("Egyszerű választás")
	}


	QCollapsible {
		id: collapsible
		title: qsTr("Egyszerű választás 2")

		QGridLayout {
			width: parent.width

			watchModification: false

			QGridLabel { field: textObjectiveModuleName2 }

			QGridTextField {
				id: textObjectiveModuleName2
				fieldName: qsTr("Hi")
			}

		}
	}


	function reloadData() {
		if (!moduleData)
			return
	}
}

