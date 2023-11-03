import QtQuick
import QtQuick.Controls

QFormSpinBox {
	text: qsTr("Feladatok száma:")

	from: 1
	to: 99
	stepSize: 1

	spin.editable: true
}
