import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.Menu {
	id: control

	x: parent.width-width
	y: parent.height

	transformOrigin: Menu.TopRight
}
