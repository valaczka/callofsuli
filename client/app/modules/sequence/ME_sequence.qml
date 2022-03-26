import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QCollapsible {
	id: control

	collapsed: false

	property var moduleData: null

	property bool editable: false

	signal modified()

	title: qsTr("Sorozat")

	rightComponent: QToolButton {
		visible: !control.editable
		icon.source: CosStyle.iconEdit
		text: qsTr("Szerkesztés")
		display: AbstractButton.IconOnly
		onClicked: control.editable = true
	}



	QGridLayout {
		id: layout

		watchModification: true
		onModifiedChanged: if (layout.modified)
							   control.modified()

		QGridLabel { field: areaItems }

		QGridTextArea {
			id: areaItems
			fieldName: qsTr("Elemek")

			placeholderText: qsTr("A sorozat elemei (soronként) növekvő sorrendben ")
			minimumHeight: CosStyle.baseHeight*3

			onTextModified: getData()
		}
	}


	Component.onCompleted: {
		if (!moduleData)
			return

		if (moduleData.items)
			areaItems.setData(moduleData.items.join("\n"))
	}


	function getData() {
		var d = {} //JS.getSqlFields([textQuestion, spinCount, spinCourrectMin, spinCourrectMax])

		d.items = areaItems.text.split("\n")

		moduleData = d

		if (editable)
			return moduleData
		else
			return {}
	}

}




