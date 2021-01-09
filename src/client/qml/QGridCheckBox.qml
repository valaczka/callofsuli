import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import "Style"

QCheckBox {
	id: control
	property string sqlField: ""
	property alias sqlData: control.checked
	property bool modified: false

	property bool watchModification: parent.watchModification

	Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
	Layout.columnSpan: parent.columns
	Layout.fillWidth: true
	Layout.bottomMargin: parent.columns === 1 ? 10 : 0


	textColor: modified ? CosStyle.colorAccent : CosStyle.colorPrimary


	onToggled: {
		if (watchModification) {
			modified = true
			parent.modified = true
		}
	}

	function setData(t) {
		checked = t ? t : false
		modified = false
	}
}
