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

	property double flowSplit: 0.35

	property list<GameQuestionDNDdrop> drops

	readonly property double implicitContentWidth: _contentItem.width-2*_contentItem.padding
	readonly property double implicitContentHeight: _contentItem.height-2*_contentItem.padding

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

			clip: true

			boundsBehavior: Flickable.StopAtBounds
			flickableDirection: Flickable.VerticalFlick

			ScrollIndicator.vertical: ScrollIndicator { active: _flick.movingVertically || _flick.contentHeight > _flick.height }

			contentWidth: _loader.item ? _loader.item.width : 0
			contentHeight: _loader.item ? _loader.item.height: 0

			Loader {
				id: _loader
			}

		}

		Qaterial.VerticalLineSeparator {
			visible: root.columns > 1
			anchors.right: parent.right
			height: parent.height*0.9
			anchors.verticalCenter: parent.verticalCenter
		}

		Qaterial.HorizontalLineSeparator {
			visible: root.columns == 1
			anchors.bottom: parent.bottom
			anchors.horizontalCenter: parent.horizontalCenter
			width: parent.width*0.9
		}
	}



	GameQuestionDNDflow {
		id: _flow
		Layout.fillWidth: !(root.columns > 1)
		Layout.preferredWidth: (root.columns > 1) ? root.parent.width*root.flowSplit : -1
		Layout.fillHeight: (root.columns > 1)
		Layout.preferredHeight: (root.columns > 1) ? -1 : root.parent.height*root.flowSplit
	}
}
