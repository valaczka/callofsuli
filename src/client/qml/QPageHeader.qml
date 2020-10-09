import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.14
import QtQuick.Controls.Material 2.3
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Rectangle {
	id: control

	width: parent.width

	color: JS.setColorAlpha(CosStyle.colorPrimaryDark, 0.2)

	height: col.height //+
			//(selectorLoader.status == Loader.Ready && selectorLoader.item.visible ? selectorLoader.item.height : 0)+1

	property bool isSelectorMode: false
	property alias labelCountText: labelCount.text

	property alias mainItem: mainItem.data

	default property alias colData: col.data

	signal selectAll()
	signal deselectAll()

	Column {
		id: col
		width: parent.width

		RowLayout {
			id: mainRow
			width: parent.width
			height: Math.max(labelCount.height, mainItem.height, buttonSelectAll.height)

			QLabel {
				id: labelCount
				text: ""

				visible: opacity != 0
				opacity: isSelectorMode ? 1 : 0

				Behavior on opacity { NumberAnimation { duration: 125 } }

				horizontalAlignment: Text.AlignHCenter
				verticalAlignment: Text.AlignVCenter

				Layout.minimumWidth: height

				Layout.fillWidth: false
				Layout.fillHeight: true
				Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
			}

			Item {
				id: mainItem

				implicitHeight: 10
				height: childrenRect.height
				Layout.fillWidth: true
				Layout.fillHeight: true
				Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
			}


			QToolButton {
				id: buttonSelectAll
				icon.source: CosStyle.iconSelectAll

				ToolTip.text: qsTr("Mindet kijelöl")

				Layout.fillWidth: false
				Layout.fillHeight: true

				visible: opacity != 0
				opacity: isSelectorMode ? 1 : 0

				Behavior on opacity { NumberAnimation { duration: 125 } }

				Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

				onClicked: control.selectAll()
			}

			/*QToolButton {
				id: buttonDeSelectAll
				icon.source: CosStyle.iconClear

				Layout.fillWidth: false
				Layout.fillHeight: true

				visible: opacity != 0
				opacity: isSelectorMode ? 1 : 0

				Behavior on opacity { NumberAnimation { duration: 125 } }

				Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

				onClicked: control.deselectAll()
			}*/
		}
	}


}

