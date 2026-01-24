import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


Item {
	id: _contentItem

	implicitHeight: 50
	implicitWidth: 50

	property double padding: 10
	property double spacing: 3

	property bool isHorizontal: false
	property bool isMonospace: false
	property bool showSeparator: false

	property int toogleMode: GameQuestionComponentImpl.ToggleSelect

	property alias repeater: rptr
	property alias model: rptr.model

	property int selectedButtonIndex: -1

	signal answerReveal(int idx, int answer)

	Flickable {
		id: _flick

		anchors.fill: parent
		anchors.margins: _contentItem.padding
		anchors.rightMargin: Math.max(_contentItem.padding, _scrollBar.width)

		clip: true

		boundsBehavior: Flickable.StopAtBounds
		flickableDirection: Flickable.VerticalFlick

		ScrollBar.vertical: ScrollBar {
			id: _scrollBar
			parent: _contentItem
			anchors.top: _flick.top
			anchors.left: _flick.right
			anchors.bottom: _flick.bottom
			policy: _flick.contentHeight > _flick.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
		}

		contentWidth: _col.width
		contentHeight: _col.height

		Column {
			id: _col

			width: _flick.width - (_scrollBar.visible ? _scrollBar.width : 0)

			y: Math.max((_flick.height-height)/2, 0)
			spacing: _contentItem.spacing

			Repeater {
				id: rptr

				delegate: GameQuestionButton {
					id: btn
					text: modelData
					width: _col.width

					implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
											 contentItem.labelItem.implicitHeight + topPadding + bottomPadding)

					maximumLineCount: 0

					fontFamily: isMonospace ? "Ubuntu Mono" : ""

					buttonType: _contentItem.selectedButtonIndex === index ? GameQuestionButton.Selected : GameQuestionButton.Neutral

					onClicked: {
						_contentItem.selectedButtonIndex = index
					}

					Connections {
						target: _contentItem

						function onAnswerReveal(idx, answer) {
							if (index === idx)
								btn.buttonType = GameQuestionButton.Correct
							else if (answer === index || selectedButtonIndex === index)
								btn.buttonType = GameQuestionButton.Wrong
						}
					}
				}
			}
		}

	}

	Qaterial.VerticalLineSeparator {
		visible: isHorizontal && !_scrollBar.visible && showSeparator
		anchors.right: parent.right
		height: parent.height*0.9
		anchors.verticalCenter: parent.verticalCenter
	}

	Qaterial.HorizontalLineSeparator {
		visible: !isHorizontal && showSeparator
		anchors.bottom: parent.bottom
		anchors.horizontalCenter: parent.horizontalCenter
		width: parent.width*0.9
	}
}


