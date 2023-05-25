import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Row {
	property bool watchModification: true
	readonly property QFormColumn _form : (parent instanceof QFormColumn) ? parent : null
	property string field: ""
	property int fieldData: -1
	readonly property alias getData: _spin.value

	property alias spin: _spin
	property alias label: _label

	property alias text: _label.text
	property alias from: _spin.from
	property alias to: _spin.to
	property alias value: _spin.value
	property alias stepSize: _spin.stepSize
	property alias validator: _spin.validator

	spacing: 5

	onFieldDataChanged: _spin.value = fieldData

	Qaterial.LabelBody1 {
		id: _label
		anchors.verticalCenter: parent.verticalCenter
	}


	SpinBox {
		id: _spin
		anchors.verticalCenter: parent.verticalCenter

		font: Qaterial.Style.textTheme.body1

		onValueModified: if (_form && watchModification) _form.modified = true
	}
}
