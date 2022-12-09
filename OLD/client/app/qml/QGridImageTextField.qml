import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import "Style"

Row {
	id: control
	Layout.fillWidth: true
	Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
	Layout.columnSpan: parent.columns
	Layout.bottomMargin: 10


	spacing: 5

	property bool watchModification: parent.watchModification

	property string fieldName: ""
	property string image: ""
	property alias text: tf.text

	property alias textfield: tf
	property alias mousearea: area

	MouseArea {
		id: area

		width: height
		height: CosStyle.twoLineHeight

		acceptedButtons: Qt.LeftButton

		Image {
			id: img
			height: parent.height*0.9
			width: height
			anchors.centerIn: parent
			fillMode: Image.PreserveAspectFit
			source: control.image
			visible: control.image.length
		}

		QFontImage {
			anchors.centerIn: parent
			height: img.height
			width: img.width
			visible: !control.image.length
			icon: CosStyle.iconBindPeople
			color: CosStyle.colorPrimaryDark
		}
	}

	QTextField {
		id: tf

		anchors.verticalCenter: parent.verticalCenter
		width: parent.width-parent.spacing-area.width

		placeholderText: fieldName

		onTextEdited:  if (watchModification) {
						   control.parent.modified = true
					   }


		onAccepted: control.parent.accept()
	}

}
