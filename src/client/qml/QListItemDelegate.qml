import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.0
import "."
import "Style"
import "JScript.js" as JS


QListView {
	id: view

	property string modelTitleRole: "labelTitle"
	property string modelSubtitleRole: ""
	property string modelEnabledRole: ""
	property string modelToolTipRole: ""
	property string modelSelectedRole: "selected"
	property string modelDepthRole: ""
	property string modelTitleColorRole: ""
	property string modelSubtitleColorRole: ""
	property string modelBackgroundRole: ""

	property bool selectorSet: false
	property bool autoSelectorChange: false

	property bool isObjectModel: isProxyModel || selectorSet || autoSelectorChange
	property bool isProxyModel: false

	property int selectedItemCount: 0

	property Component leftComponent: null
	property Component rightComponent: null

	property int delegateHeight: CosStyle.baseHeight
	property int depthWidth: CosStyle.baseHeight

	property color currentColor: CosStyle.colorPrimaryLighter

	signal clicked(int index)
	signal rightClicked(int index)
	signal longPressed(int index)

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
		property string labelSubtitle: modelSubtitleRole.length ? (
																	  isObjectModel ? model[modelSubtitleRole] :
																					  model.modelData[modelSubtitleRole]
																	  ) : ""
		property bool itemSelected: selectorSet ? (
													  isObjectModel ? model[modelSelectedRole] :
																	  model.modelData[modelSelectedRole]
													  ) : false

		property color baseColor: modelBackgroundRole.length ? (
																   isObjectModel ? model[modelBackgroundRole] : model.modelData[modelBackgroundRole]
																   ) : CosStyle.colorPrimary

		color: area.containsMouse ?
				   (currentIndex === index ? Qt.lighter(view.currentColor, 1.3) :  Qt.lighter(baseColor, 1.3)) :
				   (currentIndex === index ? view.currentColor :  baseColor)

		signal clicked()
		signal rightClicked()
		signal longPressed()

		ToolTip.text: modelToolTipRole.length ? ( isObjectModel ? model[modelToolTipRole] :
																  model.modelData[modelToolTipRole]
												 ) : ""
		ToolTip.visible: (modelToolTipRole.length ? ( isObjectModel ? model[modelToolTipRole] :
																	  model.modelData[modelToolTipRole].length)
												  : false) && (area.containsMouse || area.pressed)
		ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval

		Behavior on color { ColorAnimation { duration: 125 } }

		property var itemModel: model

		RowLayout {
			anchors.fill: parent

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


			Flipable {
				id: flipable
				width: item.height
				height: item.height

				Layout.fillHeight: false
				Layout.fillWidth: false

				visible: view.selectorSet

				property int fontSize: 24

				front: QFontImage {
					anchors.fill: parent
					icon: CosStyle.iconUnchecked
					size: flipable.fontSize
					color: CosStyle.colorAccent
				}

				back: QFontImage {
					anchors.fill: parent
					icon: CosStyle.iconChecked
					size: flipable.fontSize
					color: CosStyle.colorAccent
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
					color: modelTitleColorRole.length ? (
															isObjectModel ? model[modelTitleColorRole] : model.modelData[modelTitleColorRole]
															) : "black"
					maximumLineCount: 1
					elide: Text.ElideRight
				}

				Label {
					id: lblSubtitle
					text: item.labelSubtitle
					font.pixelSize: CosStyle.pixelSize*0.8
					font.weight: Font.Light
					color: modelSubtitleColorRole.length ? (
															   isObjectModel ? model[modelSubtitleColorRole] : model.modelData[modelSubtitleColorRole]
															   ) : "black"
				}
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
					if (view.selectorSet) {
						if (mouse.modifiers & Qt.ShiftModifier && view.currentIndex != -1) {
							var i = Math.min(view.currentIndex, index)
							var j = Math.max(view.currentIndex, index)

							for (var n=i; n<=j; ++n) {
								if (isProxyModel) {
									var idx=view.model.mapToSource(n)
									view.model.sourceModel.get(idx)[modelSelectedRole] = true
								} else {
									view.model.get(n)[modelSelectedRole] = true
								}
							}
						} else {
							model[modelSelectedRole] = !model[modelSelectedRole]
						}

						item.clicked()
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





		onClicked: {
			view.currentIndex = index
			view.clicked(index)
		}

		onLongPressed: {
			if (autoSelectorChange) {
				model[modelSelectedRole] = true
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
				model[modelSelectedRole] = !model[modelSelectedRole]
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



	function selectAll(un) {
		var t = true

		if (model.count === selectedItemCount || un === false)
			t = false

		for (var i=0; i<model.count; ++i) {
			if (isProxyModel) {
				var idx=model.mapToSource(i)
				model.sourceModel.get(idx)[modelSelectedRole] = t
			} else {
				model.get(i)[modelSelectedRole] = t
			}
		}

		calculateSelectedItems()
	}

}
