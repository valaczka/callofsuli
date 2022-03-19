import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import "Style"

RowLayout {
	id: control
	//property string fieldName: ""
	//property string sqlField: ""
	//property string sqlData: field.text+combo.textAt(combo.currentIndex)
	property bool modified: false

	property bool watchModification: parent.watchModification

	property bool acceptableInput: field1.acceptableInput && field2.acceptableInput

	property alias first: field1
	property alias second: field2
	property alias separator: separator.text
	property alias canDelete: removeButton.visible

	Layout.fillWidth: true
	Layout.bottomMargin: parent.columns === 1 ? 10 : 0

	spacing: 0

	signal modifyAction()
	signal acceptAction()
	signal deleteAction()

	QGridTextField {
		id: field1

		Layout.fillWidth: true
		Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
		Layout.bottomMargin: 0
		watchModification: false

		lineVisible: true

		onTextEdited:  {
			if (control.watchModification) {
				control.modified = true
				control.parent.modified = true
			}
		}

		onTextModified: modifyAction()

		onAccepted: field2.forceActiveFocus()
	}

	QLabel {
		id: separator
		text: "—"
		color: field1.textColor

		font.weight: Font.DemiBold

		visible: text

		leftPadding: 5
		rightPadding: 5

		Layout.fillWidth: false
		Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

	}


	QGridTextField {
		id: field2

		Layout.fillWidth: true
		Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
		Layout.bottomMargin: 0
		watchModification: false

		lineVisible: true

		onTextEdited:  {
			if (control.watchModification) {
				control.modified = true
				control.parent.modified = true
			}
		}

		onTextModified: modifyAction()

		onAccepted: acceptAction()
	}

	QToolButton {
		id: removeButton
		color: CosStyle.colorError
		icon.source: CosStyle.iconDelete

		visible: false

		Layout.fillWidth: false
		Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

		onClicked: deleteAction()
	}

	function accept() {

	}
}
