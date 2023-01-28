import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
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

	default property alias content: _content.data
	property alias contentContainer: _content
	property alias spacing: _content.spacing

	implicitHeight: _content.implicitHeight
	implicitWidth: _content.implicitWidth

	Flickable
	{
		width: parent.width
		height: Math.min(parent.height, contentHeight)
		anchors.verticalCenter: contentCentered ? parent.verticalCenter : undefined
		anchors.top: contentCentered ? undefined : parent.top

		contentHeight: _content.implicitHeight +
					   control.topPadding + control.bottomPadding +
					   (Qt.inputMethod && Qt.inputMethod.visible ?
							(Qt.inputMethod.keyboardRectangle.height / Screen.devicePixelRatio) : 0)
		flickableDirection: Flickable.AutoFlickIfNeeded

		clip: true

		Column
		{
			id: _content
			width: parent.width-control.leftPadding-control.rightPadding
			x: control.leftPadding
			y: control.topPadding
		}

		ScrollIndicator.vertical: Qaterial.ScrollIndicator {}
	}
}

