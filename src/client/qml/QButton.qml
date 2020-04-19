import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.3
import "Style"
import "JScript.js" as JS


Item {
	id: item

	implicitHeight: Math.max(rowMain.height+10+bgRectData.anchors.bottomMargin,
							 CosStyle.baseHeight)

	implicitWidth: Math.max(bgRectData.anchors.rightMargin +
							(vertical ?
								 lbl.implicitWidth :
								 (lbl.implicitWidth + leftPadding + rightPadding +
								  (mainIcon.visible ?
									   mainIcon.width+rowMain.spacing :
									   0))
							 ),
							CosStyle.baseHeight)


	width: implicitWidth
	height: implicitHeight

	property bool vertical: false
	property bool disabled: false
	property bool selected: false
	property alias label: lbl.text
	property alias labelItem: lbl
	property string icon: ""



	property color bgColor: CosStyle.colorPrimaryDark
	property color bgColorSelected: CosStyle.colorAccentDarker
	property color blinkColor: Qt.lighter(bgColor, 1.3)
	property color borderColor: CosStyle.colorPrimaryDarker
	property color textColor: "white"

	readonly property color _textColor: disabled || selected ?
											"white" :
											textColor


	readonly property color _bgColor: disabled ?
										  JS.setColorAlpha(bgColor, 0.7) :
										  selected ?
											  bgColorSelected :
											  bgColor


	property alias row: rowMain
	property int horizontalPadding: 12
	property int leftPadding: horizontalPadding
	property int rightPadding: horizontalPadding
	property bool rowCentered: false

	property alias shadow: shadowEffect.visible
	property bool animationEnabled: true



	focus: true
	activeFocusOnTab: true

	ToolTip.visible: ToolTip.text && (area.containsMouse || area.pressed)
	ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval

	onIconChanged: JS.setIconFont(mainIcon, icon)



	signal clicked()
	signal rightClicked()
	signal longPressed()


	DropShadow {
		id: shadowEffect
		anchors.fill: bgRect
		horizontalOffset: 1
		verticalOffset: 1
		radius: 3
		samples: 5
		source: bgRect
		visible: true
	}


	BorderImage {
		id: bgRectMask
		source: "qrc:/img/border.svg"
		visible: false

		anchors.fill: bgRectData

		border.left: 15; border.top: 10
		border.right: 15; border.bottom: 10

		horizontalTileMode: BorderImage.Repeat
		verticalTileMode: BorderImage.Repeat
	}

	Rectangle {
		id: bgRectData

		anchors.fill: parent
		anchors.rightMargin: shadowEffect.visible ? 2 : 0
		anchors.bottomMargin: shadowEffect.visible ? 2 : 0

		visible: false

		color: (area.containsMouse || (!animationEnabled && item.activeFocus)) ?
				   Qt.lighter(_bgColor, 1.3) :
				   _bgColor

		Behavior on color { ColorAnimation { duration: 125 } }
	}

	OpacityMask {
		id: bgRect
		source: bgRectData
		maskSource: bgRectMask
		anchors.fill: bgRectData
		opacity: 0.0
		visible: false
	}


	BorderImage {
		id: bgRectLine
		source: "qrc:/img/borderLine.svg"
		visible: false

		anchors.fill: bgRectData
		border.left: 15; border.top: 10
		border.right: 15; border.bottom: 10

		horizontalTileMode: BorderImage.Repeat
		verticalTileMode: BorderImage.Repeat
	}

	Rectangle {
		id: borderRectData
		anchors.fill: bgRectData
		visible: false
		color: item.borderColor
	}

	OpacityMask {
		id: borderRect
		source: borderRectData
		maskSource: bgRectLine
		anchors.fill: borderRectData
		visible: !item.disabled
	}


	/*DropShadow {
		anchors.fill: rowMain
		horizontalOffset: 1
		verticalOffset: 1
		radius: 1
		samples: 3
		source: rowMain
	}*/

	Grid {
		id: rowMain
		anchors.verticalCenter: bgRect.verticalCenter

		Material.elevation: 6

		x: item.leftPadding+
		   ((item.vertical || item.rowCentered) ?
				(bgRect.width-lbl.width-item.leftPadding-item.rightPadding-
				 (mainIcon.visible ? mainIcon.width : 0)-
				 (mainIcon.visible && lbl.text ? rowMain.spacing : 0)
				 )/2 :
				0)



		horizontalItemAlignment: Grid.AlignHCenter
		verticalItemAlignment: Grid.AlignVCenter

		columns: item.vertical ? 1 : 2

		spacing: 5

		readonly property real _lblMaxWidth: bgRect.width-item.leftPadding-item.rightPadding-
											 (vertical ?
												  0 :
												  (mainIcon.visible ? mainIcon.width : 0)+
												  (mainIcon.visible && lbl.text ?
													   rowMain.spacing : 0)
											  )

		Text {
			id: mainIcon
			visible: icon

			color: _textColor

			font.pointSize: lbl.font.pointSize*1.5
			opacity: disabled ? 0.3 : 1
			verticalAlignment: Qt.AlignVCenter
			horizontalAlignment: Qt.AlignHCenter
		}

		Label {
			id: lbl
			color: _textColor
			opacity: disabled ? 0.3 : 1

			font.capitalization: Font.AllUppercase
			font.weight: Font.Normal

			verticalAlignment: Qt.AlignVCenter
			horizontalAlignment: Qt.AlignHCenter

			width: Math.min(implicitWidth, rowMain._lblMaxWidth)

			elide: Text.ElideRight
			//elide: implicitWidth > rowMain._lblMaxWidth ? Text.ElideRight : Text.ElideNone
			maximumLineCount: 1

		}
	}




	MouseArea {
		id: area
		anchors.fill: bgRect
		hoverEnabled: true
		acceptedButtons: disabled ? Qt.NoButton : Qt.LeftButton | Qt.RightButton

		onClicked: {
			if (mouse.button == Qt.RightButton)
				item.rightClicked()
			else
				item.clicked()

			item.forceActiveFocus()
		}

		onPressAndHold: {
			item.longPressed()
			item.forceActiveFocus()
		}
	}



	SequentialAnimation {
		id: blinkAnimation
		loops: Animation.Infinite
		running: item.activeFocus && item.animationEnabled && !item.disabled
		alwaysRunToEnd: true

		ColorAnimation {
			target: bgRectData
			property: "color"
			to: item.blinkColor
			duration: 125
		}

		PauseAnimation {
			duration: 125
		}

		ColorAnimation {
			id: animDestColor
			target: bgRectData
			property: "color"
			to: _bgColor
			duration: 75
		}
	}

	states: [
		State {
			name: "Pressed"
			when: area.pressed && !item.disabled

			PropertyChanges {
				target: item
				scale: 0.85
			}
		}
	]

	transitions: [
		Transition {
			PropertyAnimation {
				target: item
				property: "scale"
				duration: 125
				easing.type: Easing.InOutCubic
			}
		}
	]

	SequentialAnimation {
		id: mousePressAnimation
		loops: 1
		running: false

		PropertyAction {
			target: item
			property: "state"
			value: "Pressed"
		}

		PauseAnimation {
			duration: 75
		}

		PropertyAction {
			target: item
			property: "state"
			value: ""
		}
	}
	Keys.onEnterPressed: if (!disabled) {
							 mousePressAnimation.start()
							 item.clicked()
						 }
	Keys.onReturnPressed: if (!disabled) {
							  mousePressAnimation.start()
							  item.clicked()
						  }

	Keys.onSpacePressed: if (!disabled) {
							 mousePressAnimation.start()
							 item.clicked()
						 }

	function press() {
		if (!disabled) {
			mousePressAnimation.start()
			item.clicked()
		}
	}

}

