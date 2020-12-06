import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QListView {
	id: view

	property string modelTitleRole: "labelTitle"
	property string modelSubtitleRole: ""
	property string modelEnabledRole: ""
	property string modelToolTipRole: ""
	property string modelDepthRole: ""
	property string modelTitleColorRole: ""
	property string modelSubtitleColorRole: ""
	property string modelBackgroundRole: ""
	property string modelTitleWeightRole: ""

	property int delegateHeight: CosStyle.baseHeight
	property int depthWidth: CosStyle.baseHeight

	property bool selectorSet: false
	property bool autoSelectorChange: false

	readonly property int sourceIndex: currentIndex == -1 ? -1 : model.mapToSource(currentIndex)
	readonly property VariantMapModel sourceVariantMapModel: model.sourceModel

	property Component leftComponent: null
	property Component rightComponent: null

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


	boundsBehavior: Flickable.StopAtBounds

	model: ListModel {	}

	delegate: Rectangle {
		id: item
		height: view.delegateHeight
		width: view.width - x
		x: depth*view.depthWidth

		property bool enabled: true
		property int depth: modelDepthRole.length ? model[modelDepthRole] : 0
		property string labelTitle: modelTitleRole.length ? model[modelTitleRole] : ""
		property string labelSubtitle: modelSubtitleRole.length ? model[modelSubtitleRole] : ""
		property bool itemSelected: selectorSet ? model.selected : false
		property color baseColor: modelBackgroundRole.length ? model[modelBackgroundRole] : "transparent"

		color: area.containsMouse ?
				   (currentIndex === index ? Qt.lighter(view.currentColor, 1.3) :  Qt.lighter(baseColor, 1.3)) :
				   (currentIndex === index ? view.currentColor :  baseColor)

		signal clicked(int index)
		signal rightClicked(int index)
		signal longPressed(int index)

		ToolTip.text: modelToolTipRole.length ? model[modelToolTipRole] : ""
		ToolTip.visible: (modelToolTipRole.length ? model[modelToolTipRole] : false) && (area.containsMouse || area.pressed)
		ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval

		Behavior on color { ColorAnimation { duration: 125 } }

		property var itemModel: model

		MouseArea {
			id: area
			anchors.fill: parent
			hoverEnabled: true
			acceptedButtons: Qt.LeftButton | Qt.RightButton


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


				QFlipable {
					id: flipable
					width: item.height
					height: item.height

					Layout.fillHeight: false
					Layout.fillWidth: false

					visible: view.selectorSet

					frontIcon: CosStyle.iconUnchecked
					backIcon: CosStyle.iconChecked
					color: CosStyle.colorAccent
					flipped: item.itemSelected
				}


				Column {
					Layout.fillWidth: true

					Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

					QLabel {
						id: lblTitle
						text: item.labelTitle
						color: modelTitleColorRole.length ? model[modelTitleColorRole] : CosStyle.colorPrimaryLighter
						maximumLineCount: 1
						elide: Text.ElideRight
						font.weight: modelTitleWeightRole.length ? model[modelTitleWeightRole] : Font.Normal
						width: Math.min(implicitWidth, parent.width)
					}

					QLabel {
						id: lblSubtitle
						text: item.labelSubtitle
						font.pixelSize: CosStyle.pixelSize*0.8
						font.weight: Font.Light
						color: modelSubtitleColorRole.length ? model[modelSubtitleColorRole] : CosStyle.colorPrimary
						elide: Text.ElideRight
						width: Math.min(implicitWidth, parent.width)
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



			onClicked: {
				if (mouse.button == Qt.RightButton)
					item.rightClicked(index)
				else {
					if (view.selectorSet) {
						if (mouse.modifiers & Qt.ShiftModifier && view.currentIndex != -1) {
							var i = Math.min(view.currentIndex, index)
							var j = Math.max(view.currentIndex, index)

							for (var n=i; n<=j; ++n)
								sourceVariantMapModel.select(view.model.mapToSource(n))
						} else {
							sourceVariantMapModel.selectToggle(view.model.mapToSource(index))
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
			if (autoSelectorChange) {
				sourceVariantMapModel.select(view.model.mapToSource(index))
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
			if (selectorSet)
				sourceVariantMapModel.selectToggle(view.model.mapToSource(index))
			else {
				mousePressAnimation.start()
				clicked(index)
			}

		}
	}


	highlightFollowsCurrentItem: true


	Keys.onPressed: {
		if (event.key === Qt.Key_Insert) {
			keyInsertPressed(sourceIndex)
		} else if (event.key === Qt.Key_F2) {
			keyF2Pressed(sourceIndex)
		} else if (event.key === Qt.Key_F4) {
			keyF4Pressed(sourceIndex)
		} else if (event.key === Qt.Key_Delete) {
			keyDeletePressed(sourceIndex)
		}
	}

	Connections {
		target: sourceVariantMapModel
		function onSelectedCountChanged(selectedCount) {
			if (sourceVariantMapModel.selectedCount == 0)
				selectorSet=false
		}
	}

}
