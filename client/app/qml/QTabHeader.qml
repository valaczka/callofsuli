import QtQuick 2.15
import QtQuick.Controls 2.15
import "Style"
import "JScript.js" as JS

Row {
	id: control

	width: parent.width

	property QTabContainer tabContainer: null
	property color titleColor: CosStyle.colorAccentLight
	property bool isPlaceholder: false

	topPadding: tabContainer && tabContainer.tabPage ? tabContainer.tabPage.headerPadding+(isPlaceholder ? 0 : 30) : 50
	bottomPadding: isPlaceholder ? 0 : 30
	leftPadding: CosStyle.pixelSize/2

	QFontImage {
		id: iconImage

		height: visible ? CosStyle.pixelSize*2 : 0
		width: height
		size: Math.min(height*0.8, 32)

		icon: tabContainer ? tabContainer.icon : ""

		visible: icon && !isPlaceholder

		color: titleColor
	}

	QLabel {
		id: labelTitle
		text: tabContainer ? tabContainer.title : ""
		width: control.width-iconImage.width
		font.weight: Font.Thin
		font.pixelSize: CosStyle.pixelSize*1.4
		font.capitalization: Font.AllUppercase
		elide: Text.ElideRight
		color: titleColor
		leftPadding: CosStyle.pixelSize/2
		visible: !isPlaceholder
	}

}
