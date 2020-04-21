import QtQuick 2.12
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.3
import "Style"

QTextArea {
	id: control

	property string fieldName: ""
	property string sqlField: ""
	property string sqlData: control.text
	property bool modified: false

	property bool watchModification: parent.watchModification


	placeholderText: parent.columns > 1 ? "" : fieldName

	width: parent.width

	Layout.fillWidth: true
	Layout.bottomMargin: parent.columns === 1 ? 10 : 0


	onEditingFinished:  if (watchModification) {
							   modified = true
							   parent.modified = true
						   }

	function setText(t) {
		text = t
		modified = false
	}
}


