import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3
import "Style"

QSpinBox {
	id: control
	property string sqlField: ""
	property alias sqlData: control.value
	property bool modified: false

	property bool watchModification: parent.watchModification

	Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
	Layout.fillWidth: false
	Layout.bottomMargin: parent.columns === 1 ? 10 : 0


	textColor: modified ? CosStyle.colorAccent : CosStyle.colorPrimary


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
