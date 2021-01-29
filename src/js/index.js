const lizac = require("../../build/Release/lizac.node")

document.title = "Lizac v0.1 (pre-alpha)"

let openPanel = panel => {
    for (let x of document.getElementsByClassName("menupanel")) {
        if (x.id == panel) {
            x.hidden = false;
        } else {
            x.hidden = true;
        }
    }
}


window.addEventListener('DOMContentLoaded', () => {
    lizac.initialize()

    document.getElementById('triggerBox').addEventListener('change', (event) => {
        lizac.toggleTrigger(event.currentTarget.checked)
    })

    lizac.toggleTrigger(document.getElementById('triggerBox').checked)

    document.getElementById('bhopBox').addEventListener('change', (event) => {
        lizac.toggleBhop(event.currentTarget.checked)
    })

    lizac.toggleBhop(document.getElementById('bhopBox').checked)

    document.getElementById('autostrafeBox').addEventListener('change', (event) => {
        lizac.toggleAutostrafe(event.currentTarget.checked)
    })

    lizac.toggleAutostrafe(document.getElementById('autostrafeBox').checked)

    document.getElementById('glowBox').addEventListener('change', (event) => {
        lizac.toggleGlow(event.currentTarget.checked)
    })

    lizac.toggleGlow(document.getElementById('glowBox').checked)

    setTColor(document.getElementById('tColor').value)
    setCtColor(document.getElementById('ctColor').value)
})


function hexToRgb(hex) {
    var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
    return result ? {
        r: parseInt(result[1], 16),
        g: parseInt(result[2], 16),
        b: parseInt(result[3], 16)
    } : null;
}

function setTColor(hexColor) {
    let rgb = hexToRgb(hexColor)
    lizac.setTColor(rgb.r, rgb.g, rgb.b)
}

function setCtColor(hexColor) {
    let rgb = hexToRgb(hexColor)
    lizac.setCtColor(rgb.r, rgb.g, rgb.b)
}

let setTag, tagTimeout

function setClanTagButton() {
    clearInterval(tagTimeout)
    if (document.getElementById('staticTagBox').checked) {
        setClanTag(document.getElementById('tagbox').value)
    }
    if (document.getElementById('scrollTagBox').checked || document.getElementById('buildTagBox').checked) {
        let originalTag = unescape(document.getElementById('tagbox').value)
        let currentTag = originalTag
        let direction = true
        let position = 0
        tag = () => {
            if (document.getElementById('scrollTagBox').checked) {
                let z = currentTag.substring(1)
                currentTag = z + currentTag.charAt(0)
                setClanTag(currentTag)
                tagTimeout = setTimeout(tag, document.getElementById('tagintervalbox').value)
            }
            if (document.getElementById('buildTagBox').checked) {
                if (direction) {
                    position++
                } else {
                    position--
                }
                setClanTag(originalTag.substring(0, position))
                if (position == originalTag.length || position == 0) {
                    direction = !direction
                }
                tagTimeout = setTimeout(tag, document.getElementById('tagintervalbox').value)
            }
        }
        tag()
    }
}

function setClanTag(tag) {
    if (tag != setTag) {
        let append = document.getElementById('nlTagBox').checked ? " \n" : ""
        console.log(tag + append)

        lizac.setClanTag(tag + append)
        setTag = tag
    }
    document.getElementById('tagDisplay').innerHTML = tag
}