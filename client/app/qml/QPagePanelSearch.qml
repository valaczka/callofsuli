import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.3
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS



Item {
	id: container

	width: listView ? listView.width : parent.width
	height: listView ? listView.height : parent.height
	x: listView ? listView.x : 0
	y: listView ? listView.y : 0

	clip: true

	property bool isSelectorMode: listView ? listView.selectorSet : false
	property alias labelCountText: labelCount.text
	property alias searchBar: searchBar
	property alias buttonSelectAll: buttonSelectAll

	property ListView listView: null
	property real _oldContentY: 0

	default property alias colData: col.data

	signal selectAll()

	Component {
		id: listViewHeader
		Rectangle {
			color: "transparent"
			width: listView.width
			height: control.height
		}
	}

	onListViewChanged: if (listView) {
						   listView.header = listViewHeader
						   listView.highlightFollowsCurrentItem = true

						   listView.onContentYChanged.connect(function(contY) {
							   if (listView.verticalOvershoot == 0 && container.enabled) {
								   if (listView.contentY > _oldContentY && !visibleAnimation.isHiding) {
									   container.hide()
								   } else if (listView.contentY < _oldContentY && visibleAnimation.isHiding) {
									   container.show()
								   }

								   _oldContentY = listView.contentY
							   }
						   })
					   }

	SmoothedAnimation {
		id: visibleAnimation
		target: control
		duration: 225
		easing.type: Easing.OutQuad
		property: "y"

		property bool isHiding: false
	}

	onIsSelectorModeChanged: if (isSelectorMode && enabled)
								 show()

	onEnabledChanged: if (enabled)
						  show()
					  else
						  hide()

	function show() {
		visibleAnimation.to = 0
		visibleAnimation.isHiding = false
		visibleAnimation.restart()
	}


	function hide() {
		if (enabled && listView && listView.visibleArea.heightRatio >= 1.0)
			return
		visibleAnimation.to = -control.height
		visibleAnimation.isHiding = true
		visibleAnimation.start()
	}



	Rectangle {
		id: control

		width: parent.width

		color: JS.setColorAlpha(CosStyle.colorPrimaryDark, 0.9)

		implicitHeight: Math.max(labelCount.implicitHeight, searchBar.implicitHeight, buttonSelectAll.implicitHeight, 10)
		height: col.height

		Column {
			id: col
			width: parent.width

			Row {
				id: mainRow
				width: parent.width

				QLabel {
					id: labelCount
					text: ""

					anchors.verticalCenter: parent.verticalCenter

					visible: opacity != 0
					opacity: isSelectorMode ? 1 : 0

					Behavior on opacity { NumberAnimation { duration: 125 } }

					horizontalAlignment: Text.AlignHCenter
					verticalAlignment: Text.AlignVCenter

					width: visible ? Math.max(implicitHeight, mainRow.height) : 0
				}

				QTextField {
					id: searchBar
					width: parent.width-labelCount.width-buttonSelectAll.width

					anchors.verticalCenter: parent.verticalCenter

					lineVisible: false
					clearAlwaysVisible: true

					placeholderText: qsTr("Keresés...")
				}


				QToolButton {
					id: buttonSelectAll
					icon.source: CosStyle.iconSelectAll

					anchors.verticalCenter: parent.verticalCenter

					ToolTip.text: qsTr("Mindet kijelöl")

					visible: opacity != 0
					opacity: isSelectorMode ? 1 : 0

					width: visible ? implicitWidth : 0

					Behavior on opacity { NumberAnimation { duration: 125 } }

					onClicked: container.selectAll()
				}

			}
		}

	}


	Component.onCompleted: {
		if (enabled)
			show()
		else
			hide()
	}
}

