import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import "Style"

QImageSpinBox {
	id: control

	//property string fieldName: ""					// nincs!
	property string sqlField: ""
	property var sqlData: control.value
	property bool modified: false

	property bool watchModification: parent.watchModification

	Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
	Layout.fillWidth: false
	Layout.bottomMargin: parent.columns === 1 ? 10 : 0


	textColor: watchModification ? (modified ? CosStyle.colorAccent : CosStyle.colorPrimary) : CosStyle.colorAccent
	buttonColor: watchModification ? (modified ? CosStyle.colorAccent : CosStyle.colorPrimary) : CosStyle.colorPrimary


	onValueModified: {
		if (watchModification) {
			modified = true
			parent.modified = true
		}
	}

	function setData(t) {
		value = t
		modified = false
	}
}
