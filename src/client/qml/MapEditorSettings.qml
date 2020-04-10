import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property Map map: null

	title: qsTr("Pálya adatai")

	QGridLayout {
		id: grid

		width: parent.width

		watchModification: false

		onAccepted: saveInfo()

		QGridLabel { field: textTitle }

		QGridTextField {
			id: textTitle
			fieldName: qsTr("A pálya címe")
			sqlField: "title"

			onEditingFinished: grid.saveInfo()
		}


		function saveInfo() {
			var m = JS.getSqlFields([textTitle], true)

			if (Object.keys(m).length && map) {
				map.infoUpdate(m)
			}
		}

	}

	Label {
		text: "kljlkjl"
		y: grid.height
	}


	Component.onCompleted: {
		var m = map.infoGet()
		textTitle.setText(m.title)
	}
}
