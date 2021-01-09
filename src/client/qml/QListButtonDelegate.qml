import QtQuick 2.15
import QtQuick.Controls 2.15


QListView {
	id: view
	anchors.fill: parent

	property bool labelVisible: true
	property bool buttonAnimationEnabled: true
	property int delegateHeight: CosStyle.baseHeight

	signal clicked(int index)
	signal rightClicked(int index)
	signal longPressed(int index)

	model: ListModel {	}

	delegate: QButton {
		id: delegateItem
		height: view.delegateHeight
		width: view.width

		icon: model.icon
		text: view.labelVisible ? model.label : ""
		horizontalPadding: 7

		onClicked: {
			view.currentIndex = index
			view.clicked(index)
		}
	}
}
