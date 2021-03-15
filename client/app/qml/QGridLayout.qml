import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import "Style"
import "JScript.js" as JS

Item {
	id: item

	implicitHeight: 300
	implicitWidth: 600

	property alias columns: layout.columns
	default property alias _data: layout.data

	property bool verticalCentered: false
	property bool fillHeight: false

	property alias watchModification: layout.watchModification
	property alias modified: layout.modified

	property int panelPaddingLeft: CosStyle.panelPaddingLeft
	property int panelPaddingRight: CosStyle.panelPaddingRight

	signal accepted()

	width: implicitWidth
	height: flick.contentHeight+2

	Flickable {
		id: flick

		boundsBehavior: Flickable.StopAtBounds
		flickableDirection: Flickable.VerticalFlick
		width: item.width
		height: Math.min(contentHeight+2, item.height)

		anchors.verticalCenter: item.verticalCentered ? parent.verticalCenter : undefined

		clip: true

		contentWidth: layout.width
		contentHeight: layout.height

		GridLayout {
			id: layout

			property bool watchModification: false
			property bool modified: false

			width: flick.width-item.panelPaddingLeft-item.panelPaddingRight
			x: item.panelPaddingLeft

			Binding on height {
				when: item.fillHeight
				value: item.height-2
			}

			columns: item.width > (CosStyle.pixelSize*800/18) ? 2 : 1
			columnSpacing: 5
			rowSpacing: columns > 1 ? 5 : 0

			function accept() {
				item.accepted()
			}
		}
	}


}