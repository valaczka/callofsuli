import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.3
import COS.Client 1.0
import "Style"
import "JScript.js" as JS
import "."

QToolButton {
	id: button

	property AbstractActivity activity: null

	enabled: activity && activity.canUndo > -1 && !activity.isBusy

	onClicked: undo()

	icon.source: CosStyle.iconUndo

	ToolTip.text: activity ? activity.canUndoString : ""

	Action {
		id: actionUndo
		shortcut: "Ctrl+Z"
		enabled: button.enabled
		onTriggered: button.undo()
	}

	onPressAndHold: {
		var d = JS.dialogCreateQml("List")
		d.item.title = qsTr("Visszavonás")
		d.item.simpleSelect = true
		d.item.list.modelTitleRole = "desc"
		d.item.list.modelRightRole = "id"

		/*var s = activity.db.undoStack()

		d.item.model = s.steps

		d.accepted.connect(function(idx) {
			var step = s.steps[idx].id-1
			activity.db.undo(step)
		})*/
		d.open()
	}


	function undo() {
		if (activity)
			activity.db.undo(activity.canUndo-1)
	}
}
