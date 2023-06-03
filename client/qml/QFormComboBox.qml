import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Row {
	property bool watchModification: true
	readonly property QFormColumn _form : (parent instanceof QFormColumn) ? parent : null
	property string field: ""
	property var fieldData: null
	property var getData: function() { return _combo.currentValue }

	property alias combo: _combo
	property alias label: _label

	property alias text: _label.text
	property alias model: _combo.model
	property alias currentValue: _combo.currentValue
	property alias currentIndex: _combo.currentIndex
	property alias valueRole: _combo.valueRole
	property alias textRole: _combo.textRole

	spacing: 5

	onFieldDataChanged: {
		let i = _combo.indexOfValue(fieldData)
		if (i === -1)
			_combo.currentIndex = 0
		else
			_combo.currentIndex = i
	}

	Qaterial.LabelBody1 {
		id: _label
		anchors.verticalCenter: parent.verticalCenter
	}

	Qaterial.ComboBox {
		id: _combo
		anchors.verticalCenter: parent.verticalCenter

		font: Qaterial.Style.textTheme.body2

		onActivated: if (_form && watchModification) _form.modified = true
	}
}
