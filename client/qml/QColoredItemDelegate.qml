import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

QItemDelegate {
	id: control

	property color color: Qaterial.Style.accentColor

	textColor: color
	iconColorBase: color

	outlinedIcon: true
	fillIcon: false
}
