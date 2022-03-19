import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Item {
	width: parent.width
	height: layout.height

	property var moduleData: ({})
	property var storageData: ({})
	property string storageModule: ""
	property int storageCount: 0

	QGridLayout {
		id: layout

		watchModification: false


		QGridLabel { field: textQuestion }

		QGridTextField {
			id: textQuestion
			fieldName: qsTr("Állítás")
			sqlField: "question"
			placeholderText: qsTr("Ez az állítás fog megjelenni")
		}

		QGridCheckBox {
			id: chTrue
			text: qsTr("Az állítás IGAZ")
			sqlField: "correct"
		}

	}

	Component.onCompleted: {
		if (!moduleData)
			return

		JS.setSqlFields([textQuestion, chTrue], moduleData)
	}


	function getData() {
		moduleData = JS.getSqlFields([textQuestion, chTrue])

		return moduleData
	}

}

