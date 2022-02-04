import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QTabPage {
	id: control

	activity: MapEditor {
		id: mapEditor
	}

	Column {
		anchors.centerIn: parent
		QLabel {
			id: lbl
			text: "list: %1".arg(mapEditor.editor ? mapEditor.editor.chapters.count : -1)
		}

		QButton {
			text: "load"
			onClicked: mapEditor.loadTest()
		}

		QButton {
			text: "unload"
			onClicked: mapEditor.unloadTest()
		}
	}

}
