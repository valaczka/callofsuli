import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.3
import COS.Client 1.0
import "Style"
import "JScript.js" as JS
import "."

ToolButton {
	id: button

	property AbstractDbActivity dbActivity: null

	enabled: dbActivity && dbActivity.canUndo > -1

	onClicked: undo()

	Material.foreground: CosStyle.colorPrimaryLight
	Component.onCompleted: JS.setIconFont(button, "M\ue166")

	ToolTip.text: ""
	ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
	ToolTip.visible: hovered && ToolTip.text.length

	Connections {
		target: dbActivity

		onCanUndoChanged: if (dbActivity.canUndo > -1) {
							  var t = dbActivity.undoStack()
							  button.ToolTip.text = qsTr("Visszavonás: ")+t.steps[0].desc
						  } else {
							  button.ToolTip.text = ""
						  }
	}

	onPressAndHold: {
		var d = JS.dialogCreateQml("List")
		d.item.title = qsTr("Visszavonás")
		d.item.newField.visible = false
		d.item.simpleSelect = true
		d.item.list.modelTitleRole = "desc"
		d.item.list.modelRightRole = "id"

		var s = dbActivity.undoStack()

		d.item.model = s.steps

		d.accepted.connect(function(idx) {
			var step = s.steps[idx].id-1
			dbActivity.undo(step)
		})
		d.open()
	}


	function undo() {
		if (dbActivity)
			dbActivity.undo(dbActivity.canUndo-1)
	}
}
