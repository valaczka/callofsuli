import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import "Style"

QSpinBox {
	id: control

	//property string fieldName: ""					// nincs!
	property string sqlField: ""
	property alias sqlData: control.value
	property bool modified: false

	property bool watchModification: parent.watchModification

	Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
	Layout.fillWidth: false
	Layout.bottomMargin: parent.columns === 1 ? 10 : 0


	textColor: watchModification ? (modified ? CosStyle.colorAccent : CosStyle.colorPrimary) : CosStyle.colorAccent


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
