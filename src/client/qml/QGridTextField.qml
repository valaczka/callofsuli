import QtQuick 2.12
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.3
import "Style"

QTextField {
	id: control
	property string fieldName: ""
	property string sqlField: ""
	property string sqlData: control.text
	property bool modified: false

	property bool watchModification: parent.watchModification

	//	signal applied()

	Layout.fillWidth: true

	placeholderText: parent.columns > 1 ? "" : fieldName

	Layout.bottomMargin: parent.columns === 1 ? 10 : 0


	/*QImgButton {
		id: btnApply
		visible: control.modified

		width: 18
		height: 18
		anchors.verticalCenter: parent.verticalCenter
		anchors.right: parent.right
		anchors.rightMargin: 10
		icon: "M\ue86c"
		color: cosClient.color("ok", 2)
		ToolTip.text: qsTr("Alkalmaz")

		onClicked: {
			control.applied()
			modified = false
		}
	}*/

	/*	onAccepted: {
		control.applied()
		modified = false
	}
*/

	onTextEdited: {
		if (watchModification) {
			modified = true
			parent.modified = true
		}
	}

	onAccepted: parent.accept()

	function setText(t) {
		text = t
		modified = false
		//		_watch = true
	}
}
