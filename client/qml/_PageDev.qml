import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS


Page {
	id: root

	Image {
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/internal/img/villa.png"

		cache: true
	}



	GameQuestionComponentImpl {
		id: _control

		anchors.centerIn: parent

		width: Math.min(parent.width, implicitWidth)
		height: Math.min(parent.height, implicitHeight)

		implicitHeight: 400//300 * Qaterial.Style.pixelSizeRatio
		implicitWidth: 700//_engine.count*_row.buttonSize + (_engine.count+3)*_row.spacing


		Rectangle {
			anchors.fill: parent
			color: "black"
		}

		WriterEngine {
			id: _engine

			answer: "Helyes válasz ez lesz"

			hp: _control.toggleMode ? 5 : 3

			onSucceed: _control.question.onSuccess({text: text})
			onFailed: _control.question.onFailed({text: text})

			onHpChanged: {
				if (hp > _hpRow.maxHP)
					_hpRow.maxHP = hp
			}

			Component.onCompleted: _hpRow.maxHP = hp
		}


		GameQuestionTitle {
			id: _title

			anchors.left: parent.left
			anchors.right: parent.right
			anchors.bottom: parent.bottom

			buttons: _control.toggleMode
			//buttonOkEnabled: _control.toggleMode

			title: "Teszt kérdés jön majd ide?"//questionData.question

			onButtonOkClicked: {
				let t = _answer.text
				if (t === questionData.answer.text)
					_control.question.onSuccess({text: t})
				else
					_control.question.onFailed({text: t})
			}

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
						color: Qaterial.Colors.red400
						icon: index >= _engine.hp ? Qaterial.Icons.heartOutline : Qaterial.Icons.heart
					}
				}
			}

			Rectangle {
				id: _answerRect
				border.width: 1
				border.color: Qaterial.Colors.cyan500
				color: Qaterial.Colors.black

				width: parent.width
				height: _answer.implicitHeight

				anchors.top: _hpRow.bottom
				anchors.topMargin: 15
				anchors.horizontalCenter: parent.horizontalCenter

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
					text: _engine.text
				}

			}


			Grid {
				anchors.top: _answerRect.bottom
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
					cache: false
				}

				Item {
					width: _content._isHorizontal && img.visible ? (parent.width-parent.spacing)/2 : parent.width
					height: _content._isHorizontal || !img.visible ? parent.height : (parent.height-parent.spacing)/2

					QDashboardGrid {
						id: _row

						anchors.centerIn: parent

						buttonSize: 50 * Qaterial.Style.pixelSizeRatio

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
			if (_answer.text === questionData.answer.text) {
				_answer.color = Qaterial.Colors.green400
			} else {
				_answer.text = questionData.answer.text
				_answer.color = Qaterial.Colors.red500
			}
		}


		onQuestionChanged: {
			if (storedAnswer.text !== undefined)
				_answer.text = storedAnswer.text
		}


		Keys.onPressed: {
			if (_engine.enabled)
				_engine.pressKey(event.text)
		}
	}

	Component.onCompleted: _control.forceActiveFocus()
}


