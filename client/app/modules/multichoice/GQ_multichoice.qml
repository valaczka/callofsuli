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
	required property var answerData

	property real buttonWidth: width-60

	signal succeed()
	signal failed()

	property var _buttons: []

	QLabel {
		id: labelQuestion

		font.family: "Special Elite"
		font.pixelSize: CosStyle.pixelSize*1.4
		wrapMode: Text.Wrap
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.right: btnOk.left
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
		enabled: false
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
			}
		}
	}


	Component {
		id: componentButton

		GameQuestionButton {
			width: buttonWidth

			anchors.horizontalCenter: parent.horizontalCenter

			isToggle: true

			onToggled: btnOk.enabled = true
		}
	}

	Component.onCompleted:  {
		if (!questionData || !questionData.options)
			return

		for (var i=0; i<questionData.options.length; i++) {
			var p = questionData.options[i]

			var o = componentButton.createObject(col, { text: p })
			_buttons.push(o)
		}

	}


	function answer() {
		buttonReveal()
		btnOk.enabled = false

		var success = true

		for (var i=0; i<_buttons.length; i++) {
			var p = _buttons[i]

			var correct = answerData.indices.includes(i)

			if ((correct && !p.selected) || (!correct && p.selected))
				success = false
		}

		if (success)
			succeed()
		else
			failed()
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
		else if (key === Qt.Key_7 || key === Qt.Key_G)
			buttonPressByKey(7)
		else if (key === Qt.Key_8 || key === Qt.Key_H)
			buttonPressByKey(8)
		else if (key === Qt.Key_9 || key === Qt.Key_I)
			buttonPressByKey(9)
		else if (key === Qt.Key_0) {
			for (var i=1; i<=_buttons.length; i++)
				buttonPressByKey(i)
		} else if (btnOk.enabled && (key === Qt.Key_Enter || key === Qt.Key_Return))
			btnOk.press()
	}

	function buttonPressByKey(num) {
		if (num > 0 && num-1 < _buttons.length) {
			_buttons[num-1].selected = !_buttons[num-1].selected
			btnOk.enabled = true
		}
	}

	function buttonReveal() {
		for (var i=0; i<_buttons.length; i++) {
			var btn = _buttons[i]

			btn.interactive = false

			var correct = answerData.indices.includes(i)
			btn.flipped = correct

			if (correct && btn.selected)
				btn.type = GameQuestionButton.Correct
			else if ((!correct && btn.selected) || (correct && !btn.selected))
				btn.type = GameQuestionButton.Wrong
		}

	}
}

