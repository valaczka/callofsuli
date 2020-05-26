import QtQuick 2.14
import QtQuick.Controls 2.14
import "Style"
import "JScript.js" as JS

QRectangleBg {
	id: control

	property alias tags: rptr.model
	property int maxTagWidth: width-2*tagPadding
	property int tagPadding: 5

	property color defaultColor: CosStyle.colorPrimaryLight
	property color defaultBackground: CosStyle.colorPrimaryDark

	property alias title: labelTitle.text

	property bool readOnly: false
	property bool checkable: false
	property alias checked: checkTitle.checked

//	property alias checkBox: checkTitle

	property string modelTextRole: "text"
	property string modelColorRole: ""
	property string modelBackgroundRole: ""

	height: Math.max(flow.height, labelTitle.height)

	signal clicked()


	mouseArea.acceptedButtons: Qt.LeftButton

	mouseArea.enabled: !control.readOnly

	mouseArea.onClicked: control.clicked()

	QLabel {
		id: labelTitle

		visible: !control.checkable && text.length

		anchors.top: parent.top
		anchors.topMargin: 5
		anchors.left: parent.left
	}

	Flow {
		id: flow

		anchors.left: labelTitle.visible ? labelTitle.right : checkTitle.visible ? checkTitle.right : parent.left
		anchors.right: parent.right

		topPadding: 5
		bottomPadding: 5
		spacing: 5

		Repeater {
			id: rptr
			model: ListModel {}

			Rectangle {
				id: rect

				color: modelBackgroundRole.length && model.modelData[modelBackgroundRole] ? model.modelData[modelBackgroundRole] : control.defaultBackground

				width: labelText.width+2*control.tagPadding
				height: labelText.height
				radius: height/2

				QLabel {
					id: labelText
					anchors.centerIn: parent
					maximumLineCount: 1
					font.pixelSize: CosStyle.pixelSize*0.7
					font.weight: Font.DemiBold
					color: modelColorRole.length && model.modelData[modelColorRole] ? model.modelData[modelColorRole] : control.defaultColor
					text: model.modelData[modelTextRole]
					width: control.maxTagWidth > 0 && control.maxTagWidth<implicitWidth ? control.maxTagWidth : implicitWidth
					elide: Text.ElideRight
				}
			}
		}
	}


	QCheckBox {
		id: checkTitle
		text: labelTitle.text
		visible: control.checkable
		anchors.verticalCenter: parent.verticalCenter
		anchors.left: parent.left
	}

}
