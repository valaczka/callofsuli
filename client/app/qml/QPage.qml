import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: control

	implicitWidth: 2000
	implicitHeight: 600

	property real stackTopOffset: -15

	property bool stackMode: width<implicitWidth
	property int _oldStackMode: -1

	property alias pageStack: pageStack
	property alias bgImage: bgImage
	property alias mainRow: mainRow
	property alias mainToolBar: toolbar
	property alias mainToolBarComponent: toolbarRight.sourceComponent
	property string defaultTitle: ""
	property string defaultSubTitle: ""
	property string subtitle: ""

	property alias menuButton: menuButton

	property var mainMenuFunc: null
	property var contextMenuFunc: null

	property list<Component> panelComponents

	property AbstractActivity activity: null

	readonly property bool isCurrentItem: StackView.view && StackView.view.currentItem == control


	signal pageActivated()
	signal pageDeactivated()

	onActivityChanged: if (activity) {
						   activity.client = cosClient
					   }


	background: Image {
		id: bgImage
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/internal/img/villa.png"
	}




	RowLayout {
		id: mainRow
		anchors.fill: parent
		anchors.topMargin: toolbar.height

		visible: !stackMode
	}


	onStackModeChanged: resetPanels()

	function resetPanels(forced) {
		if (!forced && !isCurrentItem && _oldStackMode != -1)
			return

		var sm = stackMode ? 1 : 0

		if (sm !== _oldStackMode || forced) {
			contextMenuFunc = null
			if (!stackMode) {
				title = defaultTitle
				subtitle = defaultSubTitle
				contextMenuFunc = null
			}

			for (var j=0; j<mainRow.children.length; j++) {
				mainRow.children[j].destroy()
			}

			pageStack.clear()

			if (stackMode) {
				if (panelComponents.length)
					addStackPanel(panelComponents[0])

			} else {

				for (var i=0; i<panelComponents.length; i++) {
					var comp = panelComponents[i]

					var obj = comp.createObject(mainRow, {
													height: parent.height,
													"Layout.fillHeight": true
												})
					if (obj === null) {
						console.error("Error creating object")
					} else if (i === 0) {
						obj.populated()
					}
				}
			}
		} else {
			if (stackMode && pageStack.currentItem)
				pageStack.currentItem.populated()
			else if (!stackMode && mainRow.children.length)
				mainRow.children[0].populated()
		}

		_oldStackMode = stackMode
	}


	StackView {
		id: pageStack
		anchors.fill: parent
		anchors.topMargin: toolbar.height+stackTopOffset

		property Page parentPage: control

		visible: stackMode
	}


	function addStackPanel(comp) {
		var obj = comp.createObject(pageStack, {
										stackMode: true,
										pageStack: pageStack
									})
		if (obj === null)
			console.warning("Component create error: ", comp)
		else
			pageStack.push(obj)

		return obj
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


	StackView.onRemoved: destroy()

	StackView.onActivated: {
		resetPanels()
		pageActivated()
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

		if (layoutBack()) {
			return true
		}

		if (pageStackBack()) {
			return true
		}

		return false
	}



	function layoutBack() {
		if (stackMode && pageStack.depth > 1) {
			pageStack.pop()
			return true
		}

		return false
	}

}
