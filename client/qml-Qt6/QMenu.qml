import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.Menu {
	id: control

	x: parent.width-width
	y: parent.height

	transformOrigin: Menu.TopRight
}
