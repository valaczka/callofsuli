function intializeStyle() {
	Qaterial.Style.theme = Qaterial.Style.Theme.Dark
	Qaterial.Style.primaryColorDark = Qt.darker(Qaterial.Colors.cyan900, 1.8)
	Qaterial.Style.accentColorDark = Qaterial.Colors.amber500
	Qaterial.Style.backgroundColorDark = "transparent"
	Qaterial.Style.foregroundColorDark = Qaterial.Colors.green200

	Qaterial.Style.iconColorDark = Qaterial.Colors.cyan400
	Qaterial.Style.primaryTextColorDark = Qaterial.Colors.cyan50

	Qaterial.Style.darkColorTheme.background = Qaterial.Colors.black
	Qaterial.Style.darkColorTheme.primary = Qaterial.Style.primaryColorDark
	Qaterial.Style.darkColorTheme.accent = Qaterial.Style.accentColorDark

	Qaterial.Style.dialogColor = Qt.darker(Qaterial.Style.primaryColorDark, 1.5)

	if (Qt.platform.os == "linux" || Qt.platform.os == "osx" || Qt.platform.os == "windows")
		Qaterial.Style.dense = true
	else
		Qaterial.Style.dense = false


	if (Qt.platform.os == "android" || Qt.platform.os == "ios") {
		Qaterial.Style.textTheme.button.weight = Font.DemiBold
		Qaterial.Style.textTheme.body2.weight = Font.Normal
		Qaterial.Style.textTheme.body2Upper.weight = Font.Normal
		Qaterial.Style.textTheme.body1.weight = Font.Medium
		Qaterial.Style.textTheme.caption.weight = Font.DemiBold
		Qaterial.Style.textTheme.overline.weight = Font.DemiBold
		Qaterial.Style.textTheme.buttonTab.weight = Font.Medium
	} else {
		Qaterial.Style.textTheme.button.weight = Font.DemiBold
		Qaterial.Style.textTheme.buttonTab.weight = Font.DemiBold
		Qaterial.Style.textTheme.body2.weight = Font.Medium
		Qaterial.Style.textTheme.body2Upper.weight = Font.Medium
		Qaterial.Style.textTheme.body1.weight = Font.Medium
		Qaterial.Style.textTheme.caption.weight = Font.DemiBold
		Qaterial.Style.textTheme.overline.weight = Font.DemiBold
		Qaterial.Style.textTheme.subtitle1.weight = Font.Medium
		Qaterial.Style.textTheme.subtitle2.weight = Font.Medium
		Qaterial.Style.textTheme.headline3.weight = Font.Normal
		Qaterial.Style.textTheme.headline4.weight = Font.Normal
		Qaterial.Style.textTheme.headline5.weight = Font.Medium
		Qaterial.Style.textTheme.headline5Upper.weight = Font.Medium
		Qaterial.Style.textTheme.headline6.weight = Font.Medium
	}


	Qaterial.Style.textTheme.subtitle2.letterSpacing = 0
	Qaterial.Style.textTheme.body1.letterSpacing = 0
	Qaterial.Style.textTheme.body2.letterSpacing = 0
	Qaterial.Style.textTheme.caption.letterSpacing = 0
	Qaterial.Style.textTheme.overline.letterSpacing = 0.75
	Qaterial.Style.textTheme.button.letterSpacing = 0.75

	Qaterial.Style.textTheme.headline1.pixelSize = Qt.binding(function() { return 72*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.headline2.pixelSize = Qt.binding(function() { return 54*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.headline3.pixelSize = Qt.binding(function() { return 42*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.headline4.pixelSize = Qt.binding(function() { return 34*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.headline5.pixelSize = Qt.binding(function() { return 24*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.headline5Upper.pixelSize = Qt.binding(function() { return 24*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.headline6.pixelSize = Qt.binding(function() { return 20*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.subtitle1.pixelSize = Qt.binding(function() { return 18*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.subtitle2.pixelSize = Qt.binding(function() { return 16*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.body1.pixelSize = Qt.binding(function() { return 18*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.button.pixelSize = Qt.binding(function() { return 15*Qaterial.Style.pixelSizeRatio })

	if (Qt.platform.os == "android" || Qt.platform.os == "ios") {
		Qaterial.Style.textTheme.body2.pixelSize = Qt.binding(function() { return 14*Qaterial.Style.pixelSizeRatio })
		Qaterial.Style.textTheme.body2Upper.pixelSize = Qt.binding(function() { return 14*Qaterial.Style.pixelSizeRatio })
		Qaterial.Style.textTheme.buttonTab.pixelSize = Qt.binding(function() { return 14*Qaterial.Style.pixelSizeRatio })
		Qaterial.Style.textTheme.caption.pixelSize = Qt.binding(function() { return 12*Qaterial.Style.pixelSizeRatio })
		Qaterial.Style.textTheme.overline.pixelSize = Qt.binding(function() { return 12*Qaterial.Style.pixelSizeRatio })
		Qaterial.Style.textTheme.hint1.pixelSize = Qt.binding(function() { return 11*Qaterial.Style.pixelSizeRatio })
		Qaterial.Style.textTheme.hint2.pixelSize = Qt.binding(function() { return 11*Qaterial.Style.pixelSizeRatio })
	} else {
		Qaterial.Style.textTheme.body2.pixelSize = Qt.binding(function() { return 15*Qaterial.Style.pixelSizeRatio })
		Qaterial.Style.textTheme.body2Upper.pixelSize = Qt.binding(function() { return 15*Qaterial.Style.pixelSizeRatio })
		Qaterial.Style.textTheme.buttonTab.pixelSize = Qt.binding(function() { return 15*Qaterial.Style.pixelSizeRatio })
		Qaterial.Style.textTheme.caption.pixelSize = Qt.binding(function() { return 13*Qaterial.Style.pixelSizeRatio })
		Qaterial.Style.textTheme.overline.pixelSize = Qt.binding(function() { return 13*Qaterial.Style.pixelSizeRatio })
		Qaterial.Style.textTheme.hint1.pixelSize = Qt.binding(function() { return 12*Qaterial.Style.pixelSizeRatio })
		Qaterial.Style.textTheme.hint2.pixelSize = Qt.binding(function() { return 12*Qaterial.Style.pixelSizeRatio })
	}

	Qaterial.Style.tabButton.minHeight = Qt.binding(function() { return (Qaterial.Style.dense ? 30 : 48) * Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.tabButton.iconWidth = Qt.binding(function() { return (Qaterial.Style.dense ? 18 : 24) * Qaterial.Style.pixelSizeRatio })

	Qaterial.Style.rawButton.minWidth = Qt.binding(function() { return (Qaterial.Style.dense ? 52 : 64) * Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.rawButton.minHeight = Qt.binding(function() { return (Qaterial.Style.dense ? 24 : 36) * Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.rawButton.iconWidth = Qt.binding(function() { return (Qaterial.Style.dense ? 14 : 18) * Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.rawButton.outlinedFocusWidth = 1

	Qaterial.Style.toolButton.minWidth = Qt.binding(function() { return (Qaterial.Style.dense ? 28 : 36) * Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.toolButton.minHeight = Qt.binding(function() { return (Qaterial.Style.dense ? 28 : 36) * Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.toolButton.iconWidth = Qt.binding(function() { return (Qaterial.Style.dense ? 18 : 24) * Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.toolButton.appBarButtonWidth = Qt.binding(function() { return (Qaterial.Style.dense ? 24 : 30) * Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.toolButton.appBarButtonHeight = Qt.binding(function() { return (Qaterial.Style.dense ? 24 : 30) * Qaterial.Style.pixelSizeRatio })

	Qaterial.Style.fab.iconWidth = Qt.binding(function() { return (Qaterial.Style.dense ? 18 : 24) * Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.menuItem.iconWidth = Qt.binding(function() { return (Qaterial.Style.dense ? 18 : 24) * Qaterial.Style.pixelSizeRatio })

	Qaterial.Style.radioIndicator.implicitWidth = Qt.binding(function() { return (Qaterial.Style.dense ? 16 : 20) * Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.radioIndicator.implicitHeight = Qt.binding(function() { return (Qaterial.Style.dense ? 16 : 20) * Qaterial.Style.pixelSizeRatio })

	Qaterial.Style.checkIndicator.implicitWidth = Qt.binding(function() { return (Qaterial.Style.dense ? 16 : 18) * Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.checkIndicator.implicitHeight = Qt.binding(function() { return (Qaterial.Style.dense ? 16 : 18) * Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.checkIndicator.iconWidth = Qt.binding(function() { return (Qaterial.Style.dense ? 16 : 19) * Qaterial.Style.pixelSizeRatio })


	Qaterial.Style.colorItemGlow = Qaterial.Colors.yellow100

}





function questionDialog(_params) {
	_params.iconColor = Qaterial.Colors.orange500
	_params.textColor = Qaterial.Colors.orange500
	_params.iconFill = false
	_params.iconSize = Qaterial.Style.roundIcon.size
	_params.standardButtons = Dialog.No | Dialog.Yes

	Qaterial.DialogManager.showDialog(_params)
}




function questionDialogPlural(_list, _question, _field, _params) {
	_params.iconColor = Qaterial.Colors.orange500
	_params.textColor = Qaterial.Colors.orange500
	_params.iconFill = false
	_params.iconSize = Qaterial.Style.roundIcon.size
	_params.standardButtons = Dialog.No | Dialog.Yes

	_params.text = _question.arg(_list.length)
	_params.text += "\n" + listGetFields(_list, _field).join(", ")

	Qaterial.DialogManager.showDialog(_params)
}



function listGetFields(_list, _field) {
	var l = []

	for (var i=0; i<_list.length; ++i)
		l.push(_list[i][_field])

	return l
}




function failMessage(_msg) {
	var r = function(err) { Client.messageWarning(err, _msg) }
	return r
}
