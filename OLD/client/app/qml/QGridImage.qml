import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import "Style"

QRectangleBg {
	id: control
	Layout.fillWidth: true
	Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
	Layout.columnSpan: parent.columns
	Layout.bottomMargin: 10

	acceptedButtons: Qt.LeftButton

	implicitHeight: Math.max(CosStyle.twoLineHeight, rw.height)

	property string fieldName: ""
	property alias image: img.source
	property alias title: labelTitle.text
	property alias subtitle: labelSubtitle.text

	property alias imageImg: img
	property alias labelTitle: labelTitle
	property alias labelSubtitle: labelSubtitle
	property alias rightComponent: rightLoader.sourceComponent

	Row {
		id: rw
		anchors.centerIn: parent

		spacing: 10

		Image {
			id: img
			height: control.height*0.9
			width: control.height
			anchors.verticalCenter: parent.verticalCenter
			fillMode: Image.PreserveAspectFit
			//visible: source.length
		}

		Column {
			id: col

			anchors.verticalCenter: parent.verticalCenter

			QLabel {
				id: labelTitle
				text: control.fieldName
				maximumLineCount: 1
				elide: Text.ElideRight
				font.pixelSize: CosStyle.pixelSize
				font.weight: Font.Medium
				width: control.width
					   -(img.visible ? rw.spacing+img.width : 0)
					   -(rightLoader.visible ? rw.spacing+rightLoader.width : 0)
				visible: text.length
			}

			QLabel {
				id: labelSubtitle
				maximumLineCount: 1
				elide: Text.ElideRight
				font.pixelSize: CosStyle.pixelSize*0.75
				font.weight: Font.Normal
				width: labelTitle.width
				visible: text.length
			}
		}

		Loader {
			id: rightLoader

			anchors.verticalCenter: parent.verticalCenter
		}
	}

}
