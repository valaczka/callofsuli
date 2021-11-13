import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QCollapsible {
	id: collapsible
	title: qsTr("Igaz/hamis állítás")

	property string moduleData: ""
	property string storageData: ""
	property string storageModule: ""
	property int storageCount: 0

	interactive: false

	QGridLayout {
		watchModification: false

		QGridLabel { field: textQuestion }

		QGridTextField {
			id: textQuestion
			fieldName: qsTr("Állítás")
			sqlField: "question"
			placeholderText: qsTr("Ez az állítás fog megjelenni")

			onTextModified: getData()
		}

		QGridCheckBox {
			id: chTrue
			text: qsTr("Az állítás IGAZ")
			sqlField: "correct"

			onToggled: getData()
		}

	}


	Component.onCompleted: {
		if (moduleData == "")
			return

		JS.setSqlFields([textQuestion, chTrue], JSON.parse(moduleData))
	}


	function getData() {
		moduleData = JSON.stringify(JS.getSqlFields([textQuestion, chTrue]))

		return moduleData
	}

}

