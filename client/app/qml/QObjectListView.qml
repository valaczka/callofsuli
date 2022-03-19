import QtQuick 2.15
import QtQuick.Controls 2.15
//import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QListView {
	id: view

	property string modelTitleRole: ""
	property string modelSubtitleRole: ""
	//property string modelEnabledRole: ""
	//property string modelToolTipRole: ""
	property string modelDepthRole: ""
	property string modelTitleColorRole: ""
	property string modelSubtitleColorRole: ""
	property string modelBackgroundRole: ""
	property string modelTitleWeightRole: ""
	property string modelTitleFamilyRole: ""
	property string modelDelegateHeightRole: ""

	property bool numbered: false
	property int startNumber: 1

	property color colorTitle: CosStyle.colorPrimaryLighter
	property color colorSubtitle: CosStyle.colorPrimary

	property real pixelSizeTitle: CosStyle.pixelSize
	property real pixelSizeSubtitle: CosStyle.pixelSize*0.8

	property string fontFamilyTitle: "Rajdhani"

	property int fontWeightTitle: Font.Medium
	property int fontWeightSubtitle: Font.Normal

	property real delegateHeight: CosStyle.baseHeight
	property real depthWidth: CosStyle.baseHeight

	property bool selectorSet: false
	property bool autoSelectorChange: false
	property bool autoUnselectorChange: autoSelectorChange

	property bool mouseAreaEnabled: true

	//readonly property int sourceIndex: currentIndex == -1 ? -1 : model.mapToSource(currentIndex)
	readonly property ObjectListModel sourceObjectListModel: model.sourceModel && (model.sourceModel instanceof ObjectListModel) ? model.sourceModel : null
	readonly property ObjectListModel objectModel: sourceObjectListModel ? sourceObjectListModel :
																		   (model instanceof ObjectListModel) ? model : null

	property Component leftComponent: null
	property Component rightComponent: null
	property Component contentComponent: null

	property bool highlightCurrentItem: true

	property color currentColor: "#33EEEEEE"

	property int panelPaddingLeft: CosStyle.panelPaddingLeft
	property int panelPaddingRight: CosStyle.panelPaddingRight

	signal clicked(int index)
	signal rightClicked(int index)
	signal longPressed(int index)
	signal keyInsertPressed(int sourceIndex)
	signal keyF2Pressed(int sourceIndex)
	signal keyF4Pressed(int sourceIndex)
	signal keyDeletePressed(int sourceIndex)


	focus: true

	//boundsBehavior: Flickable.StopAtBounds

	model: ListModel {}

	delegate: Rectangle {
		id: item
		height: modelDelegateHeightRole.length ? model[modelDelegateHeightRole]
											   : view.delegateHeight
		width: view.width - x
		x: depth*view.depthWidth

		property bool enabled: true
		property int depth: modelDepthRole.length ? model[modelDepthRole]
													 : 0
		property string labelTitle: (view.numbered ? "<b>"+(index+view.startNumber)+".</b> " : "")
									+(modelTitleRole.length ? model[modelTitleRole] : "")
		property string labelSubtitle: modelSubtitleRole.length ? model[modelSubtitleRole]  : ""
		property bool itemSelected: selectorSet ? model.selected : false
		property color baseColor: modelBackgroundRole.length ? model[modelBackgroundRole]
															 : "transparent"

		color: area.containsMouse ?
				   (currentIndex === index && view.highlightCurrentItem ? Qt.lighter(view.currentColor, 1.3) :  Qt.lighter(baseColor, 1.3)) :
				   (currentIndex === index && view.highlightCurrentItem ? view.currentColor :  baseColor)

		signal clicked(int index)
		signal rightClicked(int index)
		signal longPressed(int index)

		/*ToolTip.text: modelToolTipRole.length ? model[modelToolTipRole] : ""
		ToolTip.visible: (modelToolTipRole.length ? model[modelToolTipRole] : false) && (area.containsMouse || area.pressed)
		ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval*/

		Behavior on color { ColorAnimation { duration: 125 } }

		property var itemModel: model

		MouseArea {
			id: area
			anchors.fill: parent
			hoverEnabled: true
			acceptedButtons: Qt.LeftButton | Qt.RightButton
			enabled: mouseAreaEnabled


			Row {
				anchors.fill: parent
				anchors.leftMargin: view.panelPaddingLeft
				anchors.rightMargin: view.panelPaddingRight

				Loader {
					id: leftLoader
					sourceComponent: view.leftComponent
					visible: view.leftComponent && !view.selectorSet

					/*Layout.fillHeight: false
					Layout.fillWidth: false
					Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft*/

					anchors.verticalCenter: parent.verticalCenter

					property int modelIndex: index
					property var model: item.itemModel
				}


				QFlipable {
					id: flipable
					width: item.height
					height: item.height

					/*Layout.fillHeight: false
					Layout.fillWidth: false*/

					anchors.verticalCenter: parent.verticalCenter

					visible: view.selectorSet

					mouseArea.enabled: false

					frontIcon: CosStyle.iconUnchecked
					backIcon: CosStyle.iconChecked
					color: CosStyle.colorAccent
					flipped: item.itemSelected
				}


				Column {
					//Layout.fillWidth: true

					//Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

					anchors.verticalCenter: parent.verticalCenter

					width: parent.width
						   -(leftLoader.visible ? leftLoader.width : 0)
						   -(flipable.visible ? flipable.width : 0)
						   -(rightLoader.visible ? rightLoader.width : 0)

					QLabel {
						id: lblTitle
						text: item.labelTitle
						color: modelTitleColorRole.length ? model[modelTitleColorRole]
														  : colorTitle
						maximumLineCount: 1
						elide: Text.ElideRight
						font.pixelSize: pixelSizeTitle
						font.family: modelTitleFamilyRole.length ? model[modelTitleFamilyRole]
																 : fontFamilyTitle
						font.weight: modelTitleWeightRole.length ? model[modelTitleWeightRole]
																 : fontWeightTitle
						width: parent.width
						visible: text.length
						//clip: true
					}

					QLabel {
						id: lblSubtitle
						text: item.labelSubtitle
						font.pixelSize: pixelSizeSubtitle
						font.weight: fontWeightSubtitle
						color: modelSubtitleColorRole.length ? model[modelSubtitleColorRole]
															 : colorSubtitle
						elide: Text.ElideRight
						width: parent.width
						visible: text.length
						//clip: true
					}

					Loader {
						id: contentLoader
						sourceComponent: view.contentComponent
						visible: view.contentComponent

						width: parent.width

						property int modelIndex: index
						property var model: item.itemModel
					}
				}


				Loader {
					id: rightLoader
					sourceComponent: view.rightComponent
					visible: view.rightComponent

					/*Layout.fillHeight: false
					Layout.fillWidth: false
					Layout.alignment: Qt.AlignVCenter | Qt.AlignRight */

					anchors.verticalCenter: parent.verticalCenter

					property int modelIndex: index
					property var model: item.itemModel
				}
			}



			onClicked: {
				if (mouse.button == Qt.RightButton)
					item.rightClicked(index)
				else {
					if (view.selectorSet && objectModel) {
						if (mouse.modifiers & Qt.ShiftModifier && view.currentIndex != -1) {
							var i = Math.min(view.currentIndex, index)
							var j = Math.max(view.currentIndex, index)

							for (var n=i; n<=j; ++n)
								objectModel.select(normalizedIndex(n))

						} else {
							objectModel.selectToggle(normalizedIndex(index))
						}
						view.currentIndex = index
					} else
						item.clicked(index)
				}

				item.forceActiveFocus()
			}

			onPressAndHold: {
				item.longPressed(index)
				item.forceActiveFocus()
			}
		}





		onClicked: {
			view.currentIndex = index
			view.clicked(index)
		}

		onLongPressed: {
			if (autoSelectorChange && !selectorSet && objectModel) {
				objectModel.select(normalizedIndex(index))
				selectorSet = true
				view.currentIndex = index
			} else {
				view.currentIndex = index
				view.longPressed(index)
			}
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
			clicked(index)
		}
		Keys.onReturnPressed: {
			if (!selectorSet) mousePressAnimation.start()
			clicked(index)
		}

		Keys.onSpacePressed: {
			if (selectorSet && objectModel)
				objectModel.selectToggle(normalizedIndex(index))
			else {
				mousePressAnimation.start()
				clicked(index)
			}

		}
	}


	highlightFollowsCurrentItem: true


	Keys.onPressed: {
		if (event.key === Qt.Key_Insert) {
			keyInsertPressed(normalizedIndex(currentIndex))
		} else if (event.key === Qt.Key_F2) {
			keyF2Pressed(normalizedIndex(currentIndex))
		} else if (event.key === Qt.Key_F4) {
			keyF4Pressed(normalizedIndex(currentIndex))
		} else if (event.key === Qt.Key_Delete) {
			keyDeletePressed(normalizedIndex(currentIndex))
		} /*else if (event.key === Qt.Key_PageDown) {
			var _y = visibleArea.yPosition
			_y += visibleArea.heightRatio
			contentY = Math.max(0, Math.min(contentHeight*_y, contentHeight-height))
		}*/
	}


	Connections {
		target: objectModel
		function onSelectedCountChanged() {
			if (objectModel.selectedCount == 0 && autoUnselectorChange)
				selectorSet=false
		}
	}



	function normalizedIndex(index) {
		if (model.sourceModel) {
			return model.mapToSource(index)
		}

		return index
	}

	function modelObject(index) {
		if (objectModel) {
			return objectModel.object(normalizedIndex(index))
		}

		return model.get(normalizedIndex(index))
	}
}
