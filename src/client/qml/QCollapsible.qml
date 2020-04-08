import QtQuick 2.12
import QtQuick.Controls 2.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Rectangle {
	id: control

	width: parent.width

	property int contentHeight: content.childrenRect.height
	property alias headerHeight: headerRect.height

	height: headerRect.height+(collapsed ? 0 : contentHeight)+1

	property bool collapsed: false
	property alias title: title.text

	default property alias contents: content.data

	color: "transparent"

	Rectangle {
		id: headerRect
		width: parent.width
		height: Math.max(title.implicitHeight, arrow.implicitHeight)+4

		color: "black"

		Label {
			id: title
			anchors.left: parent.left
			anchors.verticalCenter: parent.verticalCenter
			width: parent.width-arrow.width
			font.pixelSize: CosStyle.pixelSize*0.85
			font.weight: Font.DemiBold
			font.capitalization: Font.AllUppercase
			color: CosStyle.colorAccent
			elide: Text.ElideRight
		}

		Rectangle {
			id: arrowBg
			anchors.fill: arrow
			color: "white"
			opacity: 0.5
			visible: area.containsMouse
			radius: width/2
		}

		Label {
			id: arrow
			anchors.right: parent.right
			anchors.rightMargin: 2
			anchors.verticalCenter: parent.verticalCenter
			text: collapsed ? "+" : "-"
			font.pixelSize: CosStyle.pixelSize*0.85
			font.weight: Font.DemiBold
			width: height
			verticalAlignment: Text.AlignVCenter
			horizontalAlignment: Text.AlignHCenter
		}

		MouseArea {
			id: area
			anchors.fill: arrow
			hoverEnabled: true
			acceptedButtons: Qt.LeftButton

			onClicked: collapsed = !collapsed
		}
	}

	Item {
		id: content

		anchors.top: headerRect.bottom
		anchors.left: parent.left
		anchors.right: parent.right

		visible: !collapsed
	}

	Rectangle {
		anchors.bottom: parent.bottom
		width: parent.width
		height: 1
		color: CosStyle.colorAccent
	}

}

