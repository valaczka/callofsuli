function intializeStyle() {
	Qaterial.Style.devicePixelSizeCorrection = Client.getDevicePixelSizeCorrection()

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
		Qaterial.Style.textTheme.buttonTab.weight = Font.DemiBold
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
		Qaterial.Style.textTheme.buttonTab.pixelSize = Qt.binding(function() { return 12*Qaterial.Style.pixelSizeRatio })
		Qaterial.Style.textTheme.caption.pixelSize = Qt.binding(function() { return 12*Qaterial.Style.pixelSizeRatio })
		Qaterial.Style.textTheme.overline.pixelSize = Qt.binding(function() { return 12*Qaterial.Style.pixelSizeRatio })
		Qaterial.Style.textTheme.hint1.pixelSize = Qt.binding(function() { return 11*Qaterial.Style.pixelSizeRatio })
		Qaterial.Style.textTheme.hint2.pixelSize = Qt.binding(function() { return 10*Qaterial.Style.pixelSizeRatio })
	} else {
		Qaterial.Style.textTheme.body2.pixelSize = Qt.binding(function() { return 15*Qaterial.Style.pixelSizeRatio })
		Qaterial.Style.textTheme.body2Upper.pixelSize = Qt.binding(function() { return 15*Qaterial.Style.pixelSizeRatio })
		Qaterial.Style.textTheme.buttonTab.pixelSize = Qt.binding(function() { return 15*Qaterial.Style.pixelSizeRatio })
		Qaterial.Style.textTheme.caption.pixelSize = Qt.binding(function() { return 13*Qaterial.Style.pixelSizeRatio })
		Qaterial.Style.textTheme.overline.pixelSize = Qt.binding(function() { return 13*Qaterial.Style.pixelSizeRatio })
		Qaterial.Style.textTheme.hint1.pixelSize = Qt.binding(function() { return 13*Qaterial.Style.pixelSizeRatio })
		Qaterial.Style.textTheme.hint2.pixelSize = Qt.binding(function() { return 12*Qaterial.Style.pixelSizeRatio })
	}

	Qaterial.Style.tabButton.minHeight = Qt.binding(function() { return (Qaterial.Style.dense ? 24 : 24) * Qaterial.Style.pixelSizeRatio })
	Qaterial.Style.tabButton.iconWidth = Qt.binding(function() { return (Qaterial.Style.dense ? 18 : 24) * Qaterial.Style.pixelSizeRatio })

	Qaterial.Style.colorItemGlow = Qaterial.Colors.yellow100

}





function questionDialog(_params) {
	if (_params.iconColor === undefined)
		_params.iconColor = Qaterial.Colors.orange500
	if (_params.textColor === undefined)
		_params.textColor = Qaterial.Colors.orange500
	_params.iconFill = false
	_params.iconSize = Qaterial.Style.roundIcon.size
	_params.standardButtons = DialogButtonBox.No | DialogButtonBox.Yes

	Qaterial.DialogManager.showDialog(_params)
}




function questionDialogPlural(_list, _question, _field, _params) {
	if (_params.iconColor === undefined)
		_params.iconColor = Qaterial.Colors.orange500
	if (_params.textColor === undefined)
		_params.textColor = Qaterial.Colors.orange500
	_params.iconFill = false
	_params.iconSize = Qaterial.Style.roundIcon.size
	_params.standardButtons = DialogButtonBox.No | DialogButtonBox.Yes

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



function readableTimestamp(timestamp1) {
	var date1 = new Date(timestamp1)
	var date2 = new Date()

	var y1 = date1.getFullYear()
	var y2 = date2.getFullYear()
	var m1 = date1.getMonth()
	var m2 = date2.getMonth()
	var d1 = date1.getDate()
	var d2 = date2.getDate()

	var format = ""

	if (y1 == y2 && m1 == m2 && d1 == d2)
		format = "hh:mm:ss"
	else if (y1 == y2)
		format = "MMMM d. hh:mm:ss"
	else
		format = "yyyy. MMMM d. hh:mm:ss"

	return date1.toLocaleString(Qt.locale(), format)
}


function readableTimestampMin(timestamp1) {
	var date1 = new Date(timestamp1)
	var date2 = new Date()

	var y1 = date1.getFullYear()
	var y2 = date2.getFullYear()
	var m1 = date1.getMonth()
	var m2 = date2.getMonth()
	var d1 = date1.getDate()
	var d2 = date2.getDate()

	var format = ""

	if (y1 == y2 && m1 == m2 && d1 == d2)
		format = "hh:mm"
	else if (y1 == y2)
		format = "MMMM d. hh:mm"
	else
		format = "yyyy. MMMM d. hh:mm"

	return date1.toLocaleString(Qt.locale(), format)
}



function readableDate(timestamp1) {
	var date1 = new Date(timestamp1)
	var date2 = new Date()

	var y1 = date1.getFullYear()
	var y2 = date2.getFullYear()
	var m1 = date1.getMonth()
	var m2 = date2.getMonth()
	var d1 = date1.getDate()
	var d2 = date2.getDate()

	var format = ""

	if (y1 == y2 && m1 == m2 && d1 == d2)
		return qsTr("Ma")
	else if (y1 == y2)
		format = "MMMM d."
	else
		format = "yyyy. MMMM d."

	return date1.toLocaleString(Qt.locale(), format)
}


function readableDateDate(date1, format1, format2) {
	var date2 = new Date()

	var y1 = date1.getFullYear()
	var y2 = date2.getFullYear()
	var m1 = date1.getMonth()
	var m2 = date2.getMonth()
	var d1 = date1.getDate()
	var d2 = date2.getDate()

	var format = ""

	if (y1 == y2 && m1 == m2 && d1 == d2)
		return qsTr("Ma")
	else if (y1 == y2)
		format = format1
	else
		format = format2

	return date1.toLocaleString(Qt.locale(), format)
}
