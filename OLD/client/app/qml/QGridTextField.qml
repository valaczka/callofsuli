import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import "Style"

QTextField {
	id: control
	property string fieldName: ""
	property string sqlField: ""
	property var sqlData: control.text
	property bool modified: false

	property bool watchModification: parent.watchModification

	lineVisible: true

	//	signal applied()

	implicitWidth: 100

	Layout.fillWidth: true

	placeholderText: parent.columns > 1 ? "" : fieldName

	Layout.bottomMargin: parent.columns === 1 ? 10 : 0

	onTextEdited:  if (watchModification) {
					   modified = true
					   parent.modified = true
				   }

	onTextCleared: if (watchModification) {
					   modified = true
					   parent.modified = true
				   }



	onAccepted: parent.accept()

	function setData(t) {
		text = t
		modified = false
		//		_watch = true
	}
}