import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.0
import "."
import "Style"
import "JScript.js" as JS


QListView {
	id: view

	property string modelTitleRole: "labelTitle"
	property string modelDepthRole: ""
	property string modelTitleColorRole: ""
	property string modelBackgroundRole: ""

	property bool isObjectModel: !Array.isArray(model)
	property bool isProxyModel: false

	property Component leftComponent: null
	property Component rightComponent: null

	property int delegateHeight: CosStyle.baseHeight
	property int depthWidth: CosStyle.baseHeight

	property color currentColor: JS.setColorAlpha(CosStyle.colorAccentDark, 0.4)
	property color blinkColor: JS.setColorAlpha(CosStyle.colorAccentLight, 0.4)

	property int panelPaddingLeft: CosStyle.panelPaddingLeft*2
	property int panelPaddingRight: CosStyle.panelPaddingRight*2

	signal clicked(int index)
	signal rightClicked(int index)
	signal longPressed(int index)
	signal keyInsertPressed(int index)
	signal keyF2Pressed(int index)
	signal keyF4Pressed(int index)
	signal keyDeletePressed(int index)


	model: ListModel {	}

	delegate: Rectangle {
		id: item
		height: view.delegateHeight
		width: view.width - x
		x: depth*view.depthWidth

		property bool enabled: true

		property int depth: modelDepthRole.length ? (
														isObjectModel ? model[modelDepthRole] : model.modelData[modelDepthRole]
														) : 0

		property string labelTitle: modelTitleRole.length ? (
																isObjectModel ? model[modelTitleRole] : model.modelData[modelTitleRole]
																) : ""

		property color baseColor: modelBackgroundRole.length ? (
																   isObjectModel ? model[modelBackgroundRole] : model.modelData[modelBackgroundRole]
																   ) : "transparent"

		color: area.containsMouse ?
				   (currentIndex === index ? Qt.lighter(view.currentColor, 1.3) :  Qt.lighter(baseColor, 1.3)) :
				   (currentIndex === index ? view.currentColor :  baseColor)

		signal clicked()
		signal rightClicked()
		signal longPressed()

		/*SequentialAnimation {
			id: blinkAnimation
			loops: Animation.Infinite
			running: currentIndex == index
			alwaysRunToEnd: true

			ColorAnimation {
				target: item
				property: "color"
				to: view.blinkColor
				duration: 125
			}

			PauseAnimation {
				duration: 125
			}

			ColorAnimation {
				id: animDestColor
				target: item
				property: "color"
				to: item.color
				duration: 75
			}
		}*/

		property var itemModel: model

		RowLayout {
			anchors.fill: parent
			anchors.leftMargin: view.panelPaddingLeft
			anchors.rightMargin: view.panelPaddingRight

			Loader {
				id: leftLoader
				sourceComponent: view.leftComponent
				visible: view.leftComponent && !view.selectorSet

				Layout.fillHeight: false
				Layout.fillWidth: false
				Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft

				property int modelIndex: index
				property var model: item.itemModel
			}




			QLabel {
				Layout.fillWidth: true
				Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

				id: lblTitle
				text: item.labelTitle
				color: modelTitleColorRole.length ? (
														isObjectModel ? model[modelTitleColorRole] : model.modelData[modelTitleColorRole]
														) : CosStyle.colorWarningLight
				maximumLineCount: 1
				elide: Text.ElideRight

				font.family: "HVD Peace"
				font.pixelSize: CosStyle.pixelSize*1.2
			}


			Loader {
				id: rightLoader
				sourceComponent: view.rightComponent
				visible: view.rightComponent

				Layout.fillHeight: false
				Layout.fillWidth: false
				Layout.alignment: Qt.AlignVCenter | Qt.AlignRight

				property int modelIndex: index
				property var model: item.itemModel
			}
		}


		MouseArea {
			id: area
			anchors.fill: parent
			hoverEnabled: true
			acceptedButtons: Qt.LeftButton | Qt.RightButton

			onClicked: {
				if (mouse.button == Qt.RightButton)
					item.rightClicked()
				else {
					item.clicked()
				}

				item.forceActiveFocus()
			}

			onPressAndHold: {
				item.longPressed()
				item.forceActiveFocus()
			}
		}





		onClicked: {
			view.currentIndex = index
			view.clicked(index)
		}

		onLongPressed: {
			view.currentIndex = index
			view.longPressed(index)
		}

		onRightClicked: {
			view.currentIndex = index
			view.rightClicked(index)
		}

		states: [
			State {
				name: "Pressed"
				when: area.pressed

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

		Keys.onEnterPressed: {
			item.clicked()
		}
		Keys.onReturnPressed: {
			item.clicked()
		}

		Keys.onSpacePressed: {
		mousePressAnimation.start()

			item.clicked()
		}
	}

	Keys.onPressed: {
		if (event.key === Qt.Key_Insert) {
			keyInsertPressed(view.currentIndex)
		} else if (event.key === Qt.Key_F2) {
			keyF2Pressed(view.currentIndex)
		} else if (event.key === Qt.Key_F4) {
			keyF4Pressed(view.currentIndex)
		} else if (event.key === Qt.Key_Delete) {
			keyDeletePressed(view.currentIndex)
		}
	}

}
