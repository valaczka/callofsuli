import QtQuick 2.15
import QtQuick.Controls 2.15

QFormSpinBox {
	text: qsTr("Feladatok száma:")

	from: 1
	to: 99
	stepSize: 1

	spin.editable: true
}
