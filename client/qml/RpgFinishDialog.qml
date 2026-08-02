import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


RpgDialog {
	id: panel

	required property var result

	anchors.fill: parent

	active: true

	borderColor: result.success ? Qaterial.Colors.green400 : Qaterial.Colors.red400

	Qaterial.LabelBody2 {
		anchors.fill: parent
		wrapMode: Text.Wrap
		text: (result.isTutorial === true ? "TUTORIAL\n" : "---\n") + JSON.stringify(result)
	}

}

