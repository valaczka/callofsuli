import QtQuick 2.12
import QtQuick.Controls 2.12


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
		animationEnabled: view.buttonAnimationEnabled

		disabled: model.disabled
		selected: model.selected
		icon: model.icon
		label: view.labelVisible ? model.label : ""
		horizontalPadding: 7
		rowCentered: !view.labelVisible

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
	}
}
