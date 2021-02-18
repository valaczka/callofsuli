import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import "Style"

QTextArea {
	id: control

	property string fieldName: ""
	property string sqlField: ""
	property string sqlData: control.text
	property bool modified: false

	property bool watchModification: parent.watchModification
	property bool acceptableInput: true


	placeholderText: parent.columns > 1 ? "" : fieldName

	width: parent.width

	Layout.fillWidth: true
	Layout.bottomMargin: parent.columns === 1 ? 10 : 0


	onEditingFinished:  if (watchModification) {
							   modified = true
							   parent.modified = true
						   }

	function setData(t) {
		text = t
		modified = false
	}
}


