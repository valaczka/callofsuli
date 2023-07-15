import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.Menu {
	id: control

	x: parent.width-width
	y: parent.height

	transformOrigin: Menu.TopRight
}
