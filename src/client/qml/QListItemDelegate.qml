import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import "."
import "Style"
import "JScript.js" as JS


QListView {
	id: view

	property string modelTitleRole: "labelTitle"
	property string modelSubtitleRole: ""
	property string modelRightRole: ""
	property string modelEnabledRole: ""
	property string modelToolTipRole: ""
	property string modelSeparatorRole: ""
	property string modelSelectedRole: "selected"

	property bool selectorSet: false
	property bool autoSelectorChange: false

	property bool isObjectModel: selectorSet || autoSelectorChange

	property int selectedItemCount: 0

	property Component leftComponent: null

	property int delegateHeight: 48

	signal clicked(int index)
	signal rightClicked(int index)
	signal longPressed(int index)

	model: ListModel {	}

	delegate: Rectangle {
		id: item
		height: isSeparator ? view.delegateHeight/2 : view.delegateHeight
		width: view.width

		property bool enabled: true

		property string labelTitle: modelTitleRole.length ? (
																isObjectModel ? model[modelTitleRole] : model.modelData[modelTitleRole]
																) : ""
		property string labelSubtitle: modelSubtitleRole.length ? (
																	  isObjectModel ? model[modelSubtitleRole] :
																					  model.modelData[modelSubtitleRole]
																	  ) : ""
		property string labelRight: modelRightRole.length ? (
																isObjectModel ? model[modelRightRole] :
																				model.modelData[modelRightRole]
																) : ""
		property bool itemSelected: selectorSet ? (
													  isObjectModel ? model[modelSelectedRole] :
																	  model.modelData[modelSelectedRole]
													  ) : false

		property bool isSeparator: modelSeparatorRole.length && (isObjectModel ? model[modelSeparatorRole] :
																				 model.modelData[modelSeparatorRole])
								   ? true : false

		color: isSeparator ? "transparent" :
							 (area.containsMouse || item.activeFocus) ?
								 CosStyle.colorPrimaryLighter :
								 CosStyle.colorPrimary

		signal clicked()
		signal rightClicked()
		signal longPressed()

		ToolTip.text: modelToolTipRole.length ? ( isObjectModel ? model[modelToolTipRole] :
																  model.modelData[modelToolTipRole]
												 ) : ""
		ToolTip.visible: (modelToolTipRole.length ? ( isObjectModel ? model[modelToolTipRole] :
																	  model.modelData[modelToolTipRole].length)
												  : false) && (area.containsMouse || area.pressed) && !isSeparator
		ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval

		Behavior on color { ColorAnimation { duration: 125 } }

		property var itemModel: model

		RowLayout {
			anchors.fill: parent

			visible: !isSeparator

			Loader {
				id: leftLoader
				sourceComponent: view.leftComponent
				visible: view.leftComponent && !view.selectorSet

				Layout.fillHeight: false
				Layout.fillWidth: false
				Layout.alignment: Qt.AlignCenter

				property int modelIndex: index
				property var model: item.itemModel
			}


			Flipable {
				id: flipable
				width: item.height
				height: item.height

				Layout.fillHeight: false
				Layout.fillWidth: false

				visible: view.selectorSet

				property int fontSize: 24

				front: Text {
					id: txtUnselected

					anchors.fill: parent

					font.pixelSize: flipable.fontSize
					color: CosStyle.colorAccent

					horizontalAlignment: Text.AlignHCenter
					verticalAlignment: Text.AlignVCenter

					Component.onCompleted: JS.setIconFont(txtUnselected, "M\ue836")
				}

				back: Text {
					id: txtSelected

					anchors.fill: parent

					font.pixelSize: flipable.fontSize
					color: CosStyle.colorAccent

					horizontalAlignment: Text.AlignHCenter
					verticalAlignment: Text.AlignVCenter

					Component.onCompleted: JS.setIconFont(txtSelected, "M\ue86c")
				}

				transform: Rotation {
					id: rotation
					origin.x: flipable.width/2
					origin.y: flipable.height/2
					axis.x: 0; axis.y: 1; axis.z: 0
					angle: 0
				}

				states: State {
					name: "back"
					PropertyChanges { target: rotation; angle: 180 }
					when: item.itemSelected
				}

				transitions: Transition {
					NumberAnimation { target: rotation;
						property: "angle";
						duration: 225
						easing.type: Easing.InOutCubic
					}
				}
			}


			Column {
				Layout.fillWidth: true

				Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

				Label {
					id: lblTitle
					text: item.labelTitle
					color: "black"
					maximumLineCount: 1
					elide: Text.ElideRight
				}

				Label {
					id: lblSubtitle
					text: item.labelSubtitle
					color: "black"
				}
			}


			Label {
				id: lblRight

				Layout.fillWidth: false

				Layout.alignment: Qt.AlignRight | Qt.AlignBottom

				text: item.labelRight

				color: "black"
			}
		}


		MouseArea {
			id: area
			anchors.fill: parent
			hoverEnabled: true
			acceptedButtons: Qt.LeftButton | Qt.RightButton
			enabled: !isSeparator


			onClicked: {
				if (mouse.button == Qt.RightButton)
					item.rightClicked()
				else {
					if (view.selectorSet) {
						model[modelSelectedRole] = !model[modelSelectedRole]
						calculateSelectedItems()
					} else
						item.clicked()
				}

				item.forceActiveFocus()
			}

			onPressAndHold: {
				item.longPressed()
				item.forceActiveFocus()
			}
		}


		Rectangle {
			width: parent.width*0.75
			height: 2

			visible: isSeparator

			anchors.centerIn: parent
			color: CosStyle.colorPrimaryDarker
		}


		onClicked: {
			view.currentIndex = index
			view.clicked(index)
		}

		onLongPressed: {
			if (autoSelectorChange) {
				model.modelData[modelSelectedRole] = true
				selectorSet = true
				calculateSelectedItems()
			}
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
				when: !selectorSet && area.pressed

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
			if (!selectorSet) mousePressAnimation.start()
			item.clicked()
		}
		Keys.onReturnPressed: {
			if (!selectorSet) mousePressAnimation.start()
			item.clicked()
		}

		Keys.onSpacePressed: {
			if (selectorSet) {
				model.modelData[modelSelectedRole] = !model.modelData[modelSelectedRole]
				calculateSelectedItems()
			} else
				mousePressAnimation.start()

			item.clicked()
		}
	}


	onSelectedItemCountChanged: {
		if (autoSelectorChange && selectedItemCount == 0)
			selectorSet=false
	}

	function calculateSelectedItems() {
		var n = 0
		for (var i=0; i<model.count; ++i) {
			if (model.get(i)[modelSelectedRole])
				++n
		}
		selectedItemCount = n
	}
}
