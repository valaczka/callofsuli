import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

QItemDelegate {
	id: control

	property color color: Qaterial.Style.accentColor

	textColor: color
	iconColor: color

	outlinedIcon: true
	fillIcon: false
}
