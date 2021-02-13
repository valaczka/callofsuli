


function setColorAlpha(color, alpha) {
	return Qt.hsla(color.hslHue, color.hslSaturation, color.hslLightness, alpha)
}



function createObject(_url, _parent, _prop) {
	var comp = Qt.createComponent(_url, _parent)

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




function createPage(_qml, _prop) {
	var comp = Qt.createComponent("Page"+_qml+".qml")

	if (comp.status === Component.Ready) {
		_prop.opacity = 0.0

		var incubator = comp.incubateObject(mainWindow, _prop)
		if (incubator.status !== Component.Ready) {
			incubator.onStatusChanged = function(status) {
				if (status === Component.Ready) {
					console.debug("Object ", incubator.object, "ready")
					mainStack.push(incubator.object)
				} else if (status === Component.Error) {
					console.warning("Component create error: ", _qml, incubator.errorString())
				}
			}
		} else {
			console.debug("Object ", incubator.object, "incubating...")
		}

		return incubator

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
	if (type === "info" || type === "success")
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
		d.popupContent.item.text = text ? text : ""
		d.popupContent.item.details = details ? details : ""

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


function dialogCreateQml(url, params) {
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

		//obj.popupContent.source = "QDialog"+url+".qml"

		obj.popupContent.setSource("QDialog"+url+".qml", params ? params : {})

		return obj

	} else if (comp.status === Component.Error) {
		console.warn("Error loading component: ", comp.errorString())
	}

	return null
}





function createMenu(parent, comp, menuList) {
	var m = comp.createObject(parent)

	var hasPrevious = false

	for (var i=0; i<menuList.length; i++) {
		var mm = menuList[i]

		if (mm) {
			if (hasPrevious)
				m.addSeparator()

			mm(m)
			hasPrevious = true
		}
	}

	m.closed.connect(m.destroy)
	m.popup(parent, 0, parent.height)
}




function secToMMSS(sec) {
	if (Number(sec)<=0)
		return ""

	var m = Math.floor(sec/60)
	var s = sec%60

	return String(m).padStart(2, "0")+":"+String(s).padStart(2, "0")
}



function mmSStoSec(text) {
	if (!text.length)
		return 0
	var m = String(text).match(/([0-9]+):([0-9]+)/)
	return Number(m[1])*60+Number(m[2])
}





function selectAllProxyModelToggle(model) {
	var s = false
	for (var i=0; i<model.count; ++i) {
		if (!model.get(i).selected) {
			model.sourceModel.select(model.mapToSource(i))
			s = true
		}
	}

	if (!s)
		model.sourceModel.unselectAll()
}





function getSqlFields(_items) {
	var o = {}

	for (var i=0; i<_items.length; i++) {
		o[_items[i].sqlField] = _items[i].sqlData
	}

	return o
}


function getModifiedSqlFields(_items) {
	var o = {}

	for (var i=0; i<_items.length; i++) {
		if (_items[i].modified)
			o[_items[i].sqlField] = _items[i].sqlData
	}

	return o
}


function setSqlFields(_items, _data) {
	for (var i=0; i<_items.length; i++) {
		if (Object.keys(_data).includes(_items[i].sqlField))
			_items[i].setData(_data[_items[i].sqlField])
	}
}
