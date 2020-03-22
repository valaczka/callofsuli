import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import QtQuick.Controls.Material 2.3
import "Style"


QLabel {
	id: label

	property Item field: null
	property string sqlField: ""
	property string sqlData: text

	Layout.bottomMargin: parent.columns === 1 ? 10 : 0

	Layout.fillWidth: false

	Layout.alignment: parent.columns > 1 ?
						  Qt.AlignRight | Qt.AlignVCenter :
						  Qt.AlignLeft | Qt.AlignVCenter

	horizontalAlignment: parent.columns > 1 ? Text.AlignRight : Text.AlignLeft
	verticalAlignment: Text.AlignVCenter

	font.pixelSize: CosStyle.pixelSize*0.9

	color: (field && field.modified) ? CosStyle.colorAccent : CosStyle.colorPrimary

	Behavior on color {
		ColorAnimation { duration: 125 }
	}


	function setData(t) {
		text = t
	}
}
