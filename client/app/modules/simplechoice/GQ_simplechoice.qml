import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Item {
	id: control

	implicitHeight: labelQuestion.implicitHeight+col.implicitHeight+35
	implicitWidth: 700

	required property var questionData
	property bool canPostpone: false
	property int mode: GameMatch.ModeNormal


	property real buttonWidth: width-60

	signal succeed()
	signal failed()
	signal postponed()
	signal answered(var answer)

	signal buttonReveal(GameQuestionButton original)
	signal buttonPressByKey(int num)


	QButton {
		id: btnPostpone
		enabled: canPostpone
		visible: canPostpone
		anchors.verticalCenter: labelQuestion.verticalCenter
		anchors.left: parent.left
		anchors.leftMargin: 20
		icon.source: CosStyle.iconPostpone
		text: qsTr("Később")
		themeColors: CosStyle.buttonThemeOrange
		onClicked: postponed()
	}

	QLabel {
		id: labelQuestion

		font.family: "Special Elite"
		font.pixelSize: CosStyle.pixelSize*1.4
		wrapMode: Text.Wrap
		anchors.bottom: parent.bottom
		anchors.left: btnPostpone.visible ? btnPostpone.right : parent.left
		anchors.right: btnOk.visible ? btnOk.left : parent.right
		height: Math.max(implicitHeight, btnOk.height)
		topPadding: 30
		bottomPadding: 30
		leftPadding: 20
		rightPadding: 20

		horizontalAlignment: Text.AlignHCenter

		color: CosStyle.colorAccent

		textFormat: Text.RichText

		text: questionData.question
	}


	QButton {
		id: btnOk
		enabled: (control.mode == GameMatch.ModeExam)
		visible: (control.mode == GameMatch.ModeExam)
		anchors.verticalCenter: labelQuestion.verticalCenter
		anchors.right: parent.right
		anchors.rightMargin: 20
		icon.source: CosStyle.iconOK
		text: qsTr("Kész")
		themeColors: CosStyle.buttonThemeGreen
		onClicked: answer()
	}



	Item {
		anchors.right: parent.right
		anchors.left: parent.left
		anchors.top: parent.top
		anchors.bottom: labelQuestion.top
		anchors.topMargin: 15

		Flickable {
			id: flick

			width: parent.width
			height: Math.min(parent.height, flick.contentHeight)
			anchors.centerIn: parent

			clip: true

			contentWidth: col.width
			contentHeight: col.height

			boundsBehavior: Flickable.StopAtBounds
			flickableDirection: Flickable.VerticalFlick

			ScrollIndicator.vertical: ScrollIndicator { }

			Column {
				id: col

				width: flick.width
				spacing: 3

				Repeater {
					id: rptr
					model: questionData.options

					GameQuestionButton {
						id: btn
						text: modelData
						width: buttonWidth

						anchors.horizontalCenter: parent.horizontalCenter

						onClicked: answer(index, btn)
						onToggled: {
							for (var i=0; i<rptr.model.length; i++) {
								var b = rptr.itemAt(i)
								if (b !== btn)
									b.selected = false
							}
						}

						isToggle: (control.mode == GameMatch.ModeExam)


						Connections {
							target: control
							function onButtonReveal(original) {
								btn.interactive = false

								if (original === btn && index !== questionData.answer)
									btn.type = GameQuestionButton.Wrong

								if (index === questionData.answer)
									btn.type = GameQuestionButton.Correct
							}

							function onButtonPressByKey(num) {
								if (num-1 == index && btn.interactive) {
									if (isToggle) {
										btn.selected = true
										btn.toggled()
									} else {
										btn.clicked()
									}
								}
							}
						}
					}
				}
			}
		}
	}


	function answer(btnIndex, btn) {
		if (mode == GameMatch.ModeExam) {
			var idx = -1
			for (var i=0; i<rptr.model.length; i++) {
				var b = rptr.itemAt(i)
				if (b.selected)
					idx = i
			}
			answered({index: idx})
		} else {
			buttonReveal(btn)

			if (btnIndex === questionData.answer)
				succeed()
			else
				failed()
		}
	}

	function keyPressed(key) {
		if (key === Qt.Key_1 || key === Qt.Key_A)
			buttonPressByKey(1)
		else if (key === Qt.Key_2 || key === Qt.Key_B)
			buttonPressByKey(2)
		else if (key === Qt.Key_3 || key === Qt.Key_C)
			buttonPressByKey(3)
		else if (key === Qt.Key_4 || key === Qt.Key_D)
			buttonPressByKey(4)
		else if (key === Qt.Key_5 || key === Qt.Key_E)
			buttonPressByKey(5)
		else if (key === Qt.Key_6 || key === Qt.Key_F)
			buttonPressByKey(6)
	}
}

