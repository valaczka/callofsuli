import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import QtQuick.Controls.Material 2.3
import "Style"


QLabel {
	id: label

	property Item field: null
	property string forcedText: ""

	Layout.alignment: parent.columns > 1 ?
						  Qt.AlignRight | Qt.AlignVCenter :
						  Qt.AlignLeft | Qt.AlignVCenter

	font.pixelSize: CosStyle.pixelSize*0.9

	color: (field && field.modified) ? CosStyle.colorAccent : CosStyle.colorPrimary


	Connections {
		target: parent
		function onColumnsChanged(columns) {
			if (!forcedText.length && field && columns === 1 && field.text.length === 0)
				label.opacity=0.0
			else
				label.opacity=1.0
		}

	}

	Connections {
		target: field
		function onFieldNameChanged(fieldName) {
			if (!forcedText.length)
				label.text = fieldName
		}
		function onTextChanged(text) {
			if (!forcedText.length && field && parent.columns === 1 && field.text.length === 0)
				label.opacity=0.0
			else
				label.opacity=1.0
		}
	}




	Binding on color {
		when: field && !field.acceptableInput
		value: CosStyle.colorErrorLighter
	}


	Behavior on opacity {
		NumberAnimation { duration: 125 }
	}

	Behavior on color {
		ColorAnimation { duration: 125 }
	}

	Component.onCompleted: {
		if (field && !forcedText.length) {
			text = field.fieldName
			if (parent.columns === 1 && field.text.length === 0)
				opacity = 0
		} else {
			text = forcedText
		}
	}
}
