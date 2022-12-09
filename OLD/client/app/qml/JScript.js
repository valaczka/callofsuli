


function setColorAlpha(color, alpha) {
    return Qt.hsla(color.hslHue, color.hslSaturation, color.hslLightness, alpha)
}



function createObject(_url, _parent, _prop) {
    console.warn("DEPRECATED FUNCTION")
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

function dialogMessageInfoImage(image, title, text, details) {
    return dialogMessage("info", title, text, details, image)
}

function dialogMessageWarningImage(image, title, text, details) {
    return dialogMessage("warning", title, text, details, image)
}

function dialogMessageErrorImage(image, title, text, details) {
    return dialogMessage("error", title, text, details, image)
}


function dialogMessage(type, title, text, details, image) {
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
        d.popupContent.item.image = image

        d.open()

        return d
    }

    return null
}


function dialogCreate(component) {
    var comp = Qt.createComponent("QDialog.qml", mainWindow)

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
    var comp = Qt.createComponent("QDialog.qml", mainWindow)

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




function readableInterval(timestamp1, timestamp2) {
    var date1 = new Date(timestamp1)
    var date2 = new Date(timestamp2)

    var y1 = date1.getFullYear()
    var y2 = date2.getFullYear()
    var m1 = date1.getMonth()
    var m2 = date2.getMonth()
    var d1 = date1.getDate()
    var d2 = date2.getDate()

    var format2 = ""

    if (y1 == y2 && m1 == m2 && d1 == d2)
        format2 = "hh:mm"
    else if (y1 == y2 && m1 == m2)
        format2 = "d. hh:mm"
    else if (y1 == y2)
        format2 = "MMMM d. hh:mm"
    else
        format2 = "yyyy. MMMM d. hh:mm"

    return qsTr("%1 – %2").arg(date1.toLocaleString(Qt.locale(), "yyyy. MMMM d. hh:mm")).arg(date2.toLocaleString(Qt.locale(), format2))
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



function updyteBySqlFields(_object, _items) {
    for (var i=0; i<_items.length; i++) {
        _object[_items[i].sqlField] = _items[i].sqlData
    }
}


function updateByModifiedSqlFields(_object, _items) {
    for (var i=0; i<_items.length; i++) {
        if (_items[i].modified)
            _object[_items[i].sqlField] = _items[i].sqlData
    }
}




function listModelReplace(_model, _items) {
    _model.clear()
    for (var i=0; i<_items.length; i++) {
        _model.append(_items[i])
    }
}



function listModelAppend(_model, _items) {
    for (var i=0; i<_items.length; i++) {
        _model.append(_items[i])
    }
}


function listModelCopy(_model, _items) {
    _model.clear()
    for (var i=0; i<_items.count; i++) {
        _model.append(_items.get(i))
    }
}

function listModelUpdateProperty(_model, _searchProperty, _searchValue, _property, _value) {
    for (var i=0; i<_model.count; i++) {
        if (_model.get(i)[_searchProperty] === _searchValue)
            _model.setProperty(i, _property, _value)
    }
}


function listModelUpdatePropertyAll(_model, _property, _value) {
    for (var i=0; i<_model.count; i++) {
        _model.setProperty(i, _property, _value)
    }
}



function listModelReplaceAddSelected(_model, _items) {
    _model.clear()
    for (var i=0; i<_items.length; i++) {
        var o = _items[i]
        o.selected = false
        _model.append(o)
    }
}


function listModelAppendAddSelected(_model, _items) {
    for (var i=0; i<_items.length; i++) {
        var o = _items[i]
        o.selected = false
        _model.append(o)
    }
}



function listModelGetSelectedFields(_model, _field) {
    var l = []

    for (var i=0; i<_model.count; i++) {
        var o = _model.get(i)
        if (o.selected)
            l.push(o[_field])
    }

    return l
}


