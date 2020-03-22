import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import "Style"
import "JScript.js" as JS

Item {
	id: item

	implicitWidth: minimumWidth
	implicitHeight: minimumHeight

	property alias blurSource: effectsource.sourceItem
	property alias blurRect: effectsource.sourceRect
	property alias blurBrightness: brightnessEffect.brightness

	property alias panelData: panelData
	default property alias panelDataData: panelData.data

	property int minimumWidth: 400
	property int minimumHeight: 600

	property int maximumWidth: 0
	property int maximumHeight: 0

	property int horizontalPadding: 20
	property int verticalPadding: 10

	width: if (maximumWidth>0)
			   Math.min(parent.width-2*horizontalPadding, maximumWidth)
		   else
			   parent.width-2*horizontalPadding

	height: if (maximumHeight>0)
				Math.min(parent.height-2*verticalPadding, maximumHeight)
			else
				parent.height-2*verticalPadding

	x: (parent.width-width)/2
	y: (parent.height-height)/2

	states: [
		State {
			when: (parent.width-2*horizontalPadding < minimumWidth) || (parent.height-2*verticalPadding < minimumHeight)
			PropertyChanges {
				target: item
				x: 0
				y: 0
				width: parent.width
				height: parent.height
			}

			PropertyChanges {
				target: borderRect
				visible: false
			}

			PropertyChanges {
				target: opacityEffect
				visible: false
			}

			PropertyChanges {
				target: brightnessEffect
				visible: true
			}
		}
	]



	ShaderEffectSource {
		id: effectsource
		anchors.fill: parent
		sourceRect: Qt.rect(item.x, item.y, item.width, item.height)
		visible: false
	}

	FastBlur {
		id: blurEffect
		anchors.fill: effectsource
		source: effectsource
		radius: 30
		visible: false
	}

	BrightnessContrast {
		id: brightnessEffect
		anchors.fill: blurEffect
		source: blurEffect
		brightness: -0.5
		visible: false
	}

	OpacityMask {
		id: opacityEffect
		source: brightnessEffect
		maskSource: bgRect
		anchors.fill: brightnessEffect
		visible: true
	}



	BorderImage {
		id: bgRect
		source: "qrc:/img/border.svg"
		visible: false

		width: parent.width
		height: parent.height
		border.left: 15; border.top: 10
		border.right: 15; border.bottom: 10

		horizontalTileMode: BorderImage.Repeat
		verticalTileMode: BorderImage.Repeat
	}



	BorderImage {
		id: bgRectLine
		source: "qrc:/img/borderLine15.svg"
		visible: false

		anchors.fill: bgRect
		border.left: 15; border.top: 10
		border.right: 15; border.bottom: 10

		horizontalTileMode: BorderImage.Repeat
		verticalTileMode: BorderImage.Repeat
	}

	Rectangle {
		id: borderRectData
		anchors.fill: bgRect
		visible: false
		color: CosStyle.colorPrimaryDark
	}

	OpacityMask {
		id: borderRect
		source: borderRectData
		maskSource: bgRectLine
		anchors.fill: borderRectData
		visible: true
	}



	Item {
		id: panelData
		anchors.fill: parent
		anchors.margins: 10
		visible: true
	}


}
