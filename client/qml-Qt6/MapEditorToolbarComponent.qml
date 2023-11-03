import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

Row {
	id: root

	property MapEditor editor: null

	Qaterial.AppBarButton {
		icon.source: Qaterial.Icons.contentSave
		enabled: editor && editor.modified //&& !editor.autoSaved
		visible: editor && editor.modified
		ToolTip.text: qsTr("Mentés")
		onClicked: if (editor) editor.save()
	}

	Qaterial.AppBarButton {
		icon.source: Qaterial.Icons.undo
		enabled: editor && editor.undoStack.canUndo
		visible: editor && editor.map
		ToolTip.text: editor ? editor.undoStack.undoText : ""
		onClicked: if (editor) editor.undoStack.undo()
	}

	Qaterial.AppBarButton {
		icon.source: Qaterial.Icons.redo
		enabled: editor && editor.undoStack.canRedo
		visible: editor && editor.map
		ToolTip.text: editor ? editor.undoStack.redoText : ""
		onClicked: if (editor) editor.undoStack.redo()
	}

}
