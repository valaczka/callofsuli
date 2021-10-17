import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "Style"
import "JScript.js" as JS

Page {
	id: control

	implicitWidth: 800
	implicitHeight: 600

	property string defaultTitle: ""
	property string defaultSubTitle: ""
	property string subtitle: ""

	property alias bgImage: bgImage
	property alias mainToolBar: toolbar
	property alias mainToolBarComponent: toolbarRight.sourceComponent

	property alias menuButton: menuButton

	property var mainMenuFunc: null
	property var contextMenuFunc: null

	property AbstractActivity activity: null
	property Menu toolBarMenu: null

	default property alias pageContentData: pageContent.data

	readonly property bool isCurrentItem: StackView.view && StackView.view.currentItem == control

	property bool _isFirstActivation: true

	focus: true

	signal pageActivated()
	signal pageDeactivated()
	signal pageActivatedFirst()

	onActivityChanged: if (activity) {
						   activity.client = cosClient
					   }


	background: Image {
		id: bgImage
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/internal/img/villa.png"
	}



	QToolBar {
		id: toolbar

		x: 0
		y: 0
		z: 10
		width: parent.width


		title: control.title.length ? control.title : control.defaultTitle
		subtitle: control.subtitle.length ? control.subtitle : control.defaultSubTitle

		backButtonIcon: CosStyle.iconBack
		backButton.visible: true
		backButton.onClicked: mainStack.back()

		titleItem.acceptedButtons: control.toolBarMenu && control.toolBarMenu.enabled ? Qt.LeftButton : Qt.NoButton
		titleItem.mouseArea.onClicked: {
			if (control.toolBarMenu) {
				toolBarMenu.x = titleItem.x
				toolBarMenu.y = toolbar.height
				toolBarMenu.open()
			}
		}

		QToolBusyIndicator {
			implicitHeight: CosStyle.pixelSize*1.7
			implicitWidth: implicitHeight
			height: implicitHeight
			width: running ? height : 0
			Layout.alignment: Qt.AlignCenter
			running: activity && activity.isBusy
			visible: running
		}

		Loader {
			id: toolbarRight
		}

		QToolButton {
			id: menuButton

			icon.source: CosStyle.iconMenu

			visible: mainMenuFunc || contextMenuFunc

			Component {
				id: menuComponent
				QMenu {}
			}

			onClicked: JS.createMenu(menuButton, menuComponent, [contextMenuFunc, mainMenuFunc])
		}
	}

	Item {
		id: pageContent
		anchors.top: toolbar.bottom
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.right: parent.right
	}

	StackView.onRemoved: destroy()

	StackView.onActivated: {
		pageActivated()

		if (_isFirstActivation) {
			pageActivatedFirst()
			_isFirstActivation = false
		}
	}

	StackView.onDeactivated:	pageDeactivated()


	function stackBack() {
		if (mainStack.depth > control.StackView.index+1) {
			if (!mainStack.get(control.StackView.index+1).stackBack()) {
				if (mainStack.depth > control.StackView.index+1) {
					mainStack.pop(control)
				}
			}
			return true
		}

		if (pageStackBack()) {
			return true
		}

		return false
	}

}
