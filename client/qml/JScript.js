function intializeStyle() {
	Qaterial.Style.theme = Qaterial.Style.Theme.Dark
	Qaterial.Style.primaryColorDark = Qaterial.Colors.cyan700
	Qaterial.Style.accentColorDark = Qaterial.Colors.amber500
	Qaterial.Style.backgroundColor = Qaterial.Colors.black
	Qaterial.Style.foregroundColorDark = Qaterial.Colors.cyan200

	if (Qt.platform.os == "linux" || Qt.platform.os == "osx" || Qt.platform.os == "windows")
		Qaterial.Style.dense = true
	else
		Qaterial.Style.dense = false

	Qaterial.Style.textTheme.headline1.family = "Rajdhani"
	Qaterial.Style.textTheme.headline2.family = "Rajdhani"
	Qaterial.Style.textTheme.headline3.family = "Rajdhani"
	Qaterial.Style.textTheme.headline4.family = "Rajdhani"
	Qaterial.Style.textTheme.headline5.family = "Rajdhani"
	Qaterial.Style.textTheme.headline6.family = "Rajdhani"
	Qaterial.Style.textTheme.subtitle1.family = "Rajdhani"
	Qaterial.Style.textTheme.subtitle2.family = "Rajdhani"
	Qaterial.Style.textTheme.body1.family = "Rajdhani"
	Qaterial.Style.textTheme.body2.family = "Rajdhani"
	Qaterial.Style.textTheme.button.family = "Rajdhani"
	Qaterial.Style.textTheme.caption.family = "Rajdhani"
	Qaterial.Style.textTheme.overline.family = "Rajdhani"
	Qaterial.Style.textTheme.hint1.family = "Rajdhani"
	Qaterial.Style.textTheme.hint2.family = "Rajdhani"

	Qaterial.Style.textTheme.button.weight = Font.DemiBold
	Qaterial.Style.textTheme.body2.weight = Font.Medium
	Qaterial.Style.textTheme.body1.weight = Font.Medium
	Qaterial.Style.textTheme.caption.weight = Font.Medium

	Qaterial.Style.textTheme.headline1.pixelSize = Qt.binding(function() { return 96*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.headline2.pixelSize = Qt.binding(function() { return 60*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.headline3.pixelSize = Qt.binding(function() { return 48*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.headline4.pixelSize = Qt.binding(function() { return 34*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.headline5.pixelSize = Qt.binding(function() { return 24*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.headline6.pixelSize = Qt.binding(function() { return 20*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.subtitle1.pixelSize = Qt.binding(function() { return 18*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.subtitle2.pixelSize = Qt.binding(function() { return 16*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.body1.pixelSize = Qt.binding(function() { return 18*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.body2.pixelSize = Qt.binding(function() { return 15*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.button.pixelSize = Qt.binding(function() { return 15*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.caption.pixelSize = Qt.binding(function() { return 12*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.overline.pixelSize = Qt.binding(function() { return 12*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.hint1.pixelSize = Qt.binding(function() { return 11*Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.textTheme.hint2.pixelSize = Qt.binding(function() { return 11*Qaterial.Style.pixelSizeRatio })

	Qaterial.Style.menuItem.iconWidth = Qt.binding(function() { return (Qaterial.Style.dense ? 14 : 18) * Qaterial.Style.pixelSizeRatio })
}


