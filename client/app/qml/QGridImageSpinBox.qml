import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import "Style"

Item {
	id: control

	//property string fieldName: ""					// nincs!
	property string sqlField: ""
	property var sqlData: spinBox.value
	property bool modified: false

	property bool watchModification: parent.watchModification

	property alias from: spinBox.from
	property alias to: spinBox.to
	property alias value: spinBox.value
	property alias validator: spinBox.validator
	property alias imageSize: spinBox.imageSize
	property alias textFromValue: spinBox.textFromValue
	property alias valueFromText: spinBox.valueFromText
	property alias imageFromValue: spinBox.imageFromValue

	Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
	Layout.fillWidth: true
	Layout.bottomMargin: parent.columns === 1 ? 10 : 0

	implicitHeight: spinBox.implicitHeight
	implicitWidth: spinBox.implicitWidth

	QImageSpinBox {
		id: spinBox

		anchors.verticalCenter: parent.verticalCenter
		anchors.left: parent.left

		textColor: control.watchModification ? (control.modified ? CosStyle.colorAccent : CosStyle.colorPrimary) : CosStyle.colorAccent
		buttonColor: control.watchModification ? (control.modified ? CosStyle.colorAccent : CosStyle.colorPrimary) : CosStyle.colorPrimary

		onValueModified: {
			if (control.watchModification) {
				control.modified = true
				control.parent.modified = true
			}
		}
	}

	function setData(t) {
		spinBox.value = t
		modified = false
	}
}
