import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.14
import QtQuick.Controls.Material 2.3
import "Style"


QLabel {
	id: label

	property Item field: null

	Layout.fillWidth: false

	Layout.alignment: parent.columns > 1 ?
						  Qt.AlignRight | Qt.AlignVCenter :
						  Qt.AlignLeft | Qt.AlignVCenter

	font.pixelSize: CosStyle.pixelSize*0.9

	color: (field && field.modified) ? CosStyle.colorAccent : CosStyle.colorPrimary

	Behavior on color {
		ColorAnimation { duration: 125 }
	}
}
