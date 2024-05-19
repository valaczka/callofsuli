import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


GridLayout {
	id: root

	property alias contentSource: _loader.source
	property alias contentSourceComponent: _loader.sourceComponent

	property alias dndFlow: _flow

	//property double flowSplit: 0.35
	property real flowSize: columns > 1 ? width * 0.5 : height * 0.5

	property list<GameQuestionDNDdrop> drops
	property list<Item> drags

	readonly property double implicitContentWidth: _contentItem.width-_contentItem.padding
												   -Math.max(_contentItem.padding, _scrollBar.width)
	readonly property double implicitContentHeight: _contentItem.height-2*_contentItem.padding

	readonly property double availableWidth: width-_contentItem.padding
											 -Math.max(_contentItem.padding, _scrollBar.width)
	readonly property double availableHeight: height-2*_contentItem.padding

	readonly property alias contentPadding: _contentItem.padding

	columns: parent.width > parent.height ? 2 : 1

	Item {
		id: _contentItem
		Layout.fillWidth: true
		Layout.fillHeight: true

		implicitHeight: 50
		implicitWidth: 50

		property double padding: 10

		Flickable {
			id: _flick

			anchors.fill: parent
			anchors.margins: _contentItem.padding
			anchors.rightMargin: Math.max(_contentItem.padding, _scrollBar.width)

			clip: true

			boundsBehavior: Flickable.StopAtBounds
			flickableDirection: Flickable.VerticalFlick

			//ScrollIndicator.vertical: ScrollIndicator { active: _flick.movingVertically || _flick.contentHeight > _flick.height }
			ScrollBar.vertical: ScrollBar {
				id: _scrollBar
				parent: _contentItem
				anchors.top: _flick.top
				anchors.left: _flick.right
				anchors.bottom: _flick.bottom
				policy: _flick.contentHeight > _flick.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
			}

			contentWidth: _loader.item ? _loader.item.width : 0
			contentHeight: _loader.item ? _loader.item.height: 0

			Loader {
				id: _loader
			}

		}

		Qaterial.VerticalLineSeparator {
			visible: root.columns > 1 && _flow.visible && !_scrollBar.visible
			anchors.right: parent.right
			height: parent.height*0.9
			anchors.verticalCenter: parent.verticalCenter
		}

		Qaterial.HorizontalLineSeparator {
			visible: root.columns == 1 && _flow.visible
			anchors.bottom: parent.bottom
			anchors.horizontalCenter: parent.horizontalCenter
			width: parent.width*0.9
		}
	}



	GameQuestionDNDflow {
		id: _flow
		Layout.fillWidth: !(root.columns > 1)
		Layout.preferredWidth: (root.columns > 1) ? flowSize : -1
		Layout.fillHeight: (root.columns > 1)
		Layout.preferredHeight: (root.columns > 1) ? -1 : flowSize
	}

	function createDND(_cmp, _container, _prop) {
		var o = _flow.createDND(_cmp, _container, _prop)
		if (o)
			drags.push(o)

		return o
	}


	function loadFromList(_list) {
		for (var i=0; i<_list.length && i<drops.length; i++) {
			var idx = _list[i].dragIndex

			if (idx < 0)
				continue

			var drag = null

			for (var j=0; j<drags.length; ++j) {
				if (drags[j].dragIndex === idx) {
					drag = drags[j]
					break
				}
			}

			if (!drag)
				continue

			var dest = drops[i]

			if (dest.currentDrag)
				dest.currentDrag.dropBack()

			dest.dropIn(drag)
		}
	}

}
