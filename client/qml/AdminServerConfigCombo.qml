import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


Row {
	id: root

	property alias label: _label.text
	property alias model: _combo.model
	property alias field: _ip.field

	anchors.left: parent.left

	spacing: 5

	Qaterial.LabelBody1 {
		id: _label
		anchors.verticalCenter: parent.verticalCenter
	}

	Qaterial.ComboBox {
		id: _combo

		flat: false
		editable: false

		font: Qaterial.Style.textTheme.body1

		width: Math.max(implicitWidth, Qaterial.Style.pixelSizeRatio * 200)

		anchors.verticalCenter: parent.verticalCenter

		model: ListModel {
			ListElement {
				text: qsTr("Letiltva")
				value: false
			}
			ListElement {
				text: qsTr("Engedélyezve")
				value: true
			}
		}

		valueRole: "value"
		textRole: "text"
	}


	QComboBoxInPlaceButtons {
		id: _ip
		combo: _combo

		visible: true
		enabled: active
		opacity: active ? 1.0 : 0.0

		anchors.verticalCenter: parent.verticalCenter
		property string field: ""
		readonly property var getData: _combo.currentValue

		onSaveRequest: {
			var d = {}
			d[field] = getData
			Client.send(HttpConnection.ApiAdmin, "config", d)
			.done(root, function(r){
				saved()
			})
			.fail(root, function(err) {
				Client.messageWarning(err, qsTr("Módosítás sikertelen"))
				revert()
			})
		}

		onActiveChanged: checkModified()

		Component.onCompleted: {
			_items.push(_ip)
			if (field !== "" && _data[field] !== undefined) {
				set(_combo.indexOfValue(_data[field]))
			}
		}
	}
}
