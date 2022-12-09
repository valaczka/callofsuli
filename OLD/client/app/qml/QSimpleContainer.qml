import QtQuick 2.15
import QtQuick.Controls 2.15

QSwipeContainer {
	id: control

	implicitWidth: StackView.view && StackView.view.requiredWidth ? StackView.view.requiredWidth : 800
	maximumWidth: StackView.view && StackView.view.maximumPanelWidth ? StackView.view.maximumPanelWidth : -1

	isPanelVisible:  width >= implicitWidth
}
