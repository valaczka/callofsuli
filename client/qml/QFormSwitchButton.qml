import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


Qaterial.SwitchButton {
	id: control

	readonly property QFormColumn _form : (parent instanceof QFormColumn) ? parent : null

	onToggled: if (_form) _form.modified = true
}
