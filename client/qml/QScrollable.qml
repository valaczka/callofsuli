import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Item {
	id: control

	property bool contentCentered: false
	property real horizontalPadding: Qaterial.Style.horizontalPadding
	property real verticalPadding: Qaterial.Style.horizontalPadding
	property real leftPadding: Math.max(horizontalPadding, Client.safeMarginLeft)
	property real rightPadding: Math.max(horizontalPadding, Client.safeMarginRight)
	property real topPadding: Math.max(verticalPadding, Client.safeMarginTop)
	property real bottomPadding: Math.max(verticalPadding, Client.safeMarginBottom)

	property bool refreshEnabled: false

	default property alias content: _content.data
	property alias contentContainer: _content
	property alias spacing: _content.spacing

	property alias flickable: _flickable

	signal refreshRequest()

	implicitHeight: _content.implicitHeight + control.topPadding + control.bottomPadding
	implicitWidth: _content.implicitWidth + control.leftPadding + control.rightPadding

	Flickable
	{
		id: _flickable
		width: parent.width
		height: Math.min(parent.height, contentHeight)
		anchors.verticalCenter: contentCentered ? parent.verticalCenter : undefined
		anchors.top: contentCentered ? undefined : parent.top

		contentHeight: _content.implicitHeight +
					   control.topPadding + control.bottomPadding
					   + (Qt.inputMethod && Qt.inputMethod.visible ?
							  Qt.inputMethod.keyboardRectangle.height > 0 ?
								  (Qt.inputMethod.keyboardRectangle.height / Screen.devicePixelRatio) : control.height*0.8
						  : 0)

		/* + (Qt.inputMethod && Qt.inputMethod.visible ?
							(Qt.inputMethod.keyboardRectangle.height / Screen.devicePixelRatio) : 0)*/
		flickableDirection: Flickable.VerticalFlick

		boundsBehavior: control.refreshEnabled ? Flickable.DragAndOvershootBounds : Flickable.StopAtBounds
		boundsMovement: Flickable.StopAtBounds


		clip: true

		Column
		{
			id: _content
			width: parent.width-control.leftPadding-control.rightPadding
			x: control.leftPadding
			y: control.topPadding
		}

		PullToRefreshHandler {
			target: _flickable
			enabled: control.refreshEnabled
			absoluteThreshold: 100
			onPullDownRelease: control.refreshRequest()
		}

		ScrollIndicator.vertical: Qaterial.ScrollIndicator {}
	}
}



