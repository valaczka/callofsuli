

function setIconFont(d, _icon) {
	var f = _icon.charAt(0)
	var c = _icon.charAt(1)

	if (f === 'S')
		d.font.family = "School"
	else if (f === 'B')
		d.font.family = "Books"
	else if (f === 'A')
		d.font.family = "Academic"
	else if (f === 'I')
		d.font.family = "AcademicI"
	else
		d.font.family = "Material Icons"

	d.text = c
}



function setColorAlpha(color, alpha) {
	return Qt.hsla(color.hslHue, color.hslSaturation, color.hslLightness, alpha)
}


function createPage(_qml, _prop, _parent) {
	var comp = Qt.createComponent("Page"+_qml+".qml")

	if (comp.status === Component.Ready) {
		var obj = comp.createObject(_parent, _prop)
		if (obj === null) {
			console.error("Error creating object")
			return null
		}

		mainStack.push(obj)
		return obj
	} else if (comp.status === Component.Error) {
		console.warn("Error loading component: ", comp.errorString())
	}

	return null
}



function createPageOnly(_qml, _prop, _parent) {
	var comp = Qt.createComponent("Page"+_qml+".qml")

	if (comp.status === Component.Ready) {
		var obj = comp.createObject(_parent, _prop)
		if (obj === null) {
			console.error("Error creating object")
			return null
		}

		return obj
	} else if (comp.status === Component.Error) {
		console.warn("Error loading component: ", comp.errorString())
	}

	return null
}




function dialogMessageInfo(title, text, details) {
	return dialogMessage("info", title, text, details)
}

function dialogMessageWarning(title, text, details) {
	return dialogMessage("warning", title, text, details)
}

function dialogMessageError(title, text, details) {
	return dialogMessage("error", title, text, details)
}


function dialogMessage(type, title, text, details) {
	if (type === "info")
		console.info(title+": "+text+" ("+details+")")
	else if (type === "warning")
		console.warn(title+": "+text+" ("+details+")")
	else if (type === "error")
		console.error(title+": "+text+" ("+details+")")
	else {
		console.warn("Invalid message: "+type)
		return
	}

	var c = Qt.createComponent("QDialogMessage.qml", mainWindow)

	var d = dialogCreate(c)

	if (d) {
		d.popupContent.item.type = type
		d.popupContent.item.title = title
		d.popupContent.item.text = text
		d.popupContent.item.details = details

		d.open()

		return d
	}

	return null
}


function dialogCreate(component) {
	var comp = Qt.createComponent("QDialogMain.qml", mainWindow)

	if (comp.status === Component.Ready) {
		var obj = comp.createObject(mainWindow)
		if (obj === null) {
			console.error("Error creating dialog object")
			return null
		}


		obj.x = 0
		obj.x = 0
		obj.width = mainWindow.width
		obj.height = mainWindow.height
		obj.effectSource = mainStack

		obj.popupContent.sourceComponent = component

		return obj

	} else if (comp.status === Component.Error) {
		console.warn("Error loading component: ", comp.errorString())
	}

	return null
}






function selectIndices(_model, _items) {
	for (var i=0; i<_model.count; i++) {
		var m = _model.get(i)
		m.selected = (_items.indexOf(i) > -1)
	}
}

function getSelectedIndices(model) {
	var l = []
	for (var i=0; i<model.count; i++) {
		if (model.get(i).selected === true)
			l.push(i)
	}

	return l
}



function setModel(_model, _data) {
	_model.clear()
	for (var i=0; i<_data.length; i++) {
		_model.append(_data[i])
	}
}





function getSqlFields(_items, _all) {
	var o = {}

	for (var i=0; i<_items.length; i++) {
		if (_all === true || _items[i].modified === true) {
			o[_items[i].sqlField] = _items[i].sqlData
		}
	}

	return o
}
