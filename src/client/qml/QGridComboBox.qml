import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import "Style"

QComboBox {
	id: control
	property string sqlField: ""
	property alias sqlData: control.currentValue
	property bool modified: false

	property bool watchModification: parent.watchModification

	Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
	Layout.fillWidth: false
	Layout.bottomMargin: parent.columns === 1 ? 10 : 0

	//sizeToContents: false

	onActivated: {
		if (watchModification) {
			modified = true
			parent.modified = true
		}
	}

	function setData(t) {
		var i = indexOfValue(t)
		if (i === -1)
			currentIndex = 0
		else
			currentIndex = i
		modified = false
	}
}
