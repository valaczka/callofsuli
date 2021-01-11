import QtQuick 2.15
import QtQuick.Controls 2.15
import "Style"
import "JScript.js" as JS


Rectangle {
	id: control

	width: Math.max(100, label.implicitWidth)
	height: Math.max(40, label.implicitHeight)

	radius: 3

	enum ButtonType {
		Neutral,
		Correct,
		Wrong
	}

	property bool interactive: true
	property int type: GameQuestionButton.Neutral

	property color textColor: CosStyle.buttonThemeDefault[0]
	property color backgroundColor: CosStyle.buttonThemeDefault[1]
	property color borderColor: CosStyle.buttonThemeDefault[2]
	property alias label: label
	property alias text: label.text



	signal clicked()

	border.width: 1
	border.color: if (interactive) {
					  borderColor
				  } else if (type === GameQuestionButton.Neutral) {
					  CosStyle.buttonThemeDefault[2]
				  } else if (type === GameQuestionButton.Correct) {
					  CosStyle.buttonThemeApply[2]
				  } else {
					  CosStyle.buttonThemeDelete[2]
				  }

	opacity: interactive || type !== GameQuestionButton.Neutral ? 1.0 : 0.4
	color: if (interactive) {
			   backgroundColor
		   } else if (type === GameQuestionButton.Neutral) {
			   CosStyle.buttonThemeDefault[1]
		   } else if (type === GameQuestionButton.Correct) {
			   CosStyle.buttonThemeApply[1]
		   } else {
			   CosStyle.buttonThemeDelete[1]
		   }

	Behavior on color {
		ColorAnimation { duration: 175 }
	}

	Behavior on opacity {
		NumberAnimation { duration: 175 }
	}

	QLabel {
		id: label
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.verticalCenter: parent.verticalCenter
		topPadding: 5
		bottomPadding: 5
		leftPadding: 10
		rightPadding: 10
		horizontalAlignment: Text.AlignHCenter
		verticalAlignment: Text.AlignVCenter
		wrapMode: Text.Wrap

		font.pixelSize: CosStyle.pixelSize
		font.weight: Font.DemiBold

		color: if (interactive) {
				   textColor
			   } else if (type === GameQuestionButton.Neutral) {
				   CosStyle.buttonThemeDefault[0]
			   } else if (type === GameQuestionButton.Correct) {
				   CosStyle.buttonThemeApply[0]
			   } else {
				   CosStyle.buttonThemeDelete[0]
			   }

		Behavior on color {
			ColorAnimation { duration: 175 }
		}
	}

	scale: area.pressed ? 0.85 : 1.0

	Behavior on scale {
		NumberAnimation {
			duration: 125
			easing.type: Easing.OutQuad
		}
	}

	MouseArea {
		id: area
		anchors.fill: parent
		enabled: control.interactive
		acceptedButtons: Qt.LeftButton
		onClicked: {
			control.clicked()
		}
	}
}
