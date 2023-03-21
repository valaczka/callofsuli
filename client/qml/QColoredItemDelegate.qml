import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

QItemDelegate {
	id: control

	property color color: Qaterial.Style.accentColor

	textColor: color
	iconColor: color

	outlinedIcon: true
	fillIcon: false
}
