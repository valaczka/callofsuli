import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial



GameQuestionComponentImpl {
	id: _control

	implicitHeight: (img.visible ? 600 : 400) * Qaterial.Style.pixelSizeRatio
	implicitWidth: img.visible ? 700 : (_engine.count+2)*(_row.buttonSize + _row.spacing)

	WriterEngine {
		id: _engine

		answer: questionData.answer ? questionData.answer : ""

		//hp: _control.toggleMode ? 5 : 3

		onSucceed: _control.question.onSuccess({text: text})
		onFailed: _control.question.onFailed({text: text})

		onHpChanged: {
			if (hp > _hpRow.maxHP)
				_hpRow.maxHP = hp
		}

		onWrongKeyPressed: _control.vibrateRequest()

		Component.onCompleted: _hpRow.maxHP = hp
	}


	GameQuestionTitle {
		id: _title

		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		buttons: _control.toggleMode == GameQuestionComponentImpl.ToggleSelect
		buttonOkVisible: false

		title: questionData.question
	}

	Item {
		id: _content

		anchors.right: parent.right
		anchors.left: parent.left
		anchors.top: parent.top
		anchors.bottom: _title.top
		anchors.topMargin: 15

		readonly property bool _isHorizontal: _control.width > _control.height

		Row {
			id: _hpRow
			anchors.top: parent.top
			anchors.horizontalCenter: parent.horizontalCenter
			spacing: 5

			property int maxHP: 0

			Repeater {
				model: _hpRow.maxHP

				delegate: Qaterial.Icon {
					color: _engine.hp > 0 ? Qaterial.Colors.green400 : Qaterial.Colors.red400
					icon: index >= _engine.hp ? Qaterial.Icons.heartOutline : Qaterial.Icons.heart
				}
			}
		}

		Item {
			id: _answerHolder

			anchors.top: _hpRow.bottom
			anchors.topMargin: 15
			anchors.left: parent.left
			anchors.right: parent.right
			height: img.visible ? _answerRect.height : Math.max(_answerRect.height, (parent.height-(_hpRow.y+_hpRow.height))*0.4)

			Rectangle {
				id: _answerRect
				border.width: _engine.enabled ? 1 : 0
				border.color: Qaterial.Colors.cyan500
				color: Qaterial.Colors.black

				width: parent.width-(50 * Qaterial.Style.pixelSizeRatio)
				height: _answer.implicitHeight

				anchors.centerIn: parent


				Qaterial.LabelHeadline5 {
					id: _answer
					anchors.fill: parent
					horizontalAlignment: Text.AlignLeft
					verticalAlignment: Text.AlignVCenter
					leftPadding: 10
					rightPadding: 10
					topPadding: 5
					bottomPadding: 5
					color: Qaterial.Colors.orange400
					text: _engine.displayText
				}

			}
		}


		Grid {
			anchors.top: _answerHolder.bottom
			anchors.left: parent.left
			anchors.right: parent.right
			anchors.bottom: parent.bottom
			anchors.margins: 5

			spacing: 5

			columns: _content._isHorizontal ? 2 : 1


			Image {
				id: img
				source: _control.questionData.image !== undefined ? questionData.image : ""
				visible: _control.questionData.image !== undefined

				width: _content._isHorizontal ? (parent.width-parent.spacing)/2 : parent.width
				height: _content._isHorizontal ? parent.height : (parent.height-parent.spacing)/2

				fillMode: Image.PreserveAspectFit
				cache: true
				//asynchronous: true

				sourceSize.width: width
				sourceSize.height: height
			}

			Item {
				width: _content._isHorizontal && img.visible ? (parent.width-parent.spacing)/2 : parent.width
				height: _content._isHorizontal || !img.visible ? parent.height : (parent.height-parent.spacing)/2

				QDashboardGrid {
					id: _row

					anchors.centerIn: parent

					buttonSize: 60 * Qaterial.Style.pixelSizeRatio
					spacing: 2

					horizontalPadding: 0
					topPadding: 0
					bottomPadding: 0

					enabled: _engine.enabled

					contentItems: _engine.count

					Repeater {
						model: _engine.count

						delegate: GameQuestionButton {
							id: _button

							font: Qaterial.Style.textTheme.headline6

							text: _engine.characters[index]
							width: _row.buttonSize
							height: _row.buttonSize

							buttonType: GameQuestionButton.Neutral

							Connections {
								target: _engine

								function onTextChanged() {
									_button.buttonType = GameQuestionButton.Neutral
								}

								function onWrongKeyPressed(btnIndex) {
									if (btnIndex === index)
										_button.buttonType = GameQuestionButton.Wrong
								}
							}

							onClicked: _engine.write(index)
						}
					}
				}
			}
		}





	}


	onAnswerReveal: {
		if (_answer.text === questionData.answer) {
			_answer.color = Qaterial.Colors.green400
		} else {
			_answer.text = questionData.answer
			_answer.color = Qaterial.Colors.red500
		}
	}


	onQuestionChanged: {
		if (storedAnswer.text !== undefined) {
			_engine.enabled = false
			_engine.hp = 0
			_answer.text = storedAnswer.text
		}
	}


	Keys.onPressed: event => {
		if (_engine.enabled)
			_engine.pressKey(event.text)
	}
}

