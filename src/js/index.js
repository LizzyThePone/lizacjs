const lizac = require("../../build/Release/lizac.node")
const weaponMap = require("../js/weapons")
const skinMap = require("../js/skins")


document.title = "Lizac v0.9 (beta)"

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

    lizac.toggleTrigger(document.getElementById('rcsBox').checked)

    document.getElementById('rcsBox').addEventListener('change', (event) => {
        lizac.toggleRCS(event.currentTarget.checked)
    })

    lizac.toggleRCS(document.getElementById('triggerBox').checked)

    document.getElementById('bhopBox').addEventListener('change', (event) => {
        lizac.toggleBhop(event.currentTarget.checked)
    })

    lizac.toggleBhop(document.getElementById('bhopBox').checked)

    document.getElementById('autostrafeBox').addEventListener('change', (event) => {
        lizac.toggleAutostrafe(event.currentTarget.checked)
    })

    lizac.toggleAutostrafe(document.getElementById('autostrafeBox').checked)

    document.getElementById('radarBox').addEventListener('change', (event) => {
        lizac.toggleRadar(event.currentTarget.checked)
    })

    lizac.toggleRadar(document.getElementById('radarBox').checked)

    document.getElementById('noflashBox').addEventListener('change', (event) => {
        lizac.toggleNoflash(event.currentTarget.checked)
    })

    lizac.toggleNoflash(document.getElementById('noflashBox').checked)

    document.getElementById('glowBox').addEventListener('change', (event) => {
        lizac.toggleGlow(event.currentTarget.checked)
    })

    lizac.toggleGlow(document.getElementById('glowBox').checked)

    document.getElementById('skinBox').addEventListener('change', (event) => {
        lizac.toggleSkins(event.currentTarget.checked)
    })

    lizac.toggleSkins(document.getElementById('skinBox').checked)

    setTColor(document.getElementById('tColor').value)
    setCtColor(document.getElementById('ctColor').value)

    document.getElementById('kitNameBox').onkeydown = filterSkins

    weaponMap.forEach((v, k) => {
        document.getElementById("gunlist").innerHTML = document.getElementById("gunlist").innerHTML + `<option class="gunItem" onclick="setGun()" id="gun${k}"> ${v.name} </option>`
    })

    skinMap.forEach((v, k) => {
        if (v.name == "None") {
            document.getElementById("skinlist").innerHTML = document.getElementById("skinlist").innerHTML + `<option class="skinItem" onclick="setSkin()" id="skin${k}" selected="true"> ${v.name} </option>`
        } else {
            document.getElementById("skinlist").innerHTML = document.getElementById("skinlist").innerHTML + `<option class="skinItem" onclick="setSkin()" id="skin${k}"> ${v.name} </option>`
        }

    })

    document.addEventListener("keydown", e => {
        if (triggerBinding) {
            binds.trigger.key = e.keyCode
            binds.trigger.name = e.code
            document.getElementById("triggerBind").innerHTML = e.code
            triggerBinding = false
        } else if (assistBinding) {
            let keyInfo = getvKey(e)
            binds.assist.key = keyInfo.vKey
            binds.assist.name = e.code
            document.getElementById("assistBind").innerHTML = e.code
            assistBinding = false
        }

    })

    document.addEventListener("mousedown", e => {
        if (triggerBinding) {
            let keyInfo = getvKey(e)
            binds.trigger.key = keyInfo.vKey
            binds.trigger.name = `Mouse${keyInfo.name}`
            document.getElementById("triggerBind").innerHTML = `Mouse${keyInfo.name}`
            triggerBinding = false
        }
        if (assistBinding) {
            let keyInfo = getvKey(e)
            binds.assist.key = keyInfo.vKey
            binds.assist.name = `Mouse${keyInfo.name}`
            document.getElementById("assistBind").innerHTML = `Mouse${keyInfo.name}`
            assistBinding = false
        }
    })

})

let getvKey = e => {
    let vKey, name;
    switch (e.button) {
        case 0:
            vKey = 0x01
            name = 1
            break;
        case 1:
            vKey = 0x04
            name = 3
            break;
        case 2:
            vKey = 0x02
            name = 2
            break;
        case 3:
            vKey = 0x05
            name = 5
            break;
        case 4:
            vKey = 0x06
            name = 4
            break;
    }
    return { vKey, name }

}

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
        let prepend = document.getElementById('backwardsTagBox').checked ? "\u202E" : ""
        console.log(prepend + tag + append)

        lizac.setClanTag(prepend + tag + append)
        setTag = tag
    }
    document.getElementById('tagDisplay').innerHTML = tag
}

function setGun() {
    let selectedWeaponId = parseInt(document.getElementById('gunlist').options[document.getElementById('gunlist').selectedIndex].id.replace('gun', ""))
    let weapon = weaponMap.get(selectedWeaponId)
    if (weapon.kit) {
        document.getElementById('skinlist').options[document.getElementById('skinlist').selectedIndex].selected = false
        document.getElementById(`skin${weapon.kit}`).selected = true
        document.getElementById('seedBox').value = weapon.seed
        document.getElementById('stattrakBox').value = weapon.stattrak
        document.getElementById('kitBox').value = weapon.kit
        document.getElementById('kitNameBox').value = document.getElementById(`skin${weapon.kit}`).innerHTML.trim()
    } else {
        document.getElementById('skinlist').options[document.getElementById('skinlist').selectedIndex].selected = false
        document.getElementById(`skin0`).selected = true
        document.getElementById('seedBox').value = 0
        document.getElementById('stattrakBox').value = 0
        document.getElementById('kitBox').value = 0
        document.getElementById('kitNameBox').value = document.getElementById(`skin0`).innerHTML.trim()
    }
    weaponMap.set(selectedWeaponId, weapon)
}

function setSkin() {
    let selectedWeaponId = parseInt(document.getElementById('gunlist').options[document.getElementById('gunlist').selectedIndex].id.replace('gun', ""))
    let selectedSkinId = parseInt(document.getElementById('skinlist').options[document.getElementById('skinlist').selectedIndex].id.replace('skin', ""))
    let weapon = weaponMap.get(selectedWeaponId)
    weapon.kit = selectedSkinId
    weapon.seed = (document.getElementById('seedBox').value == "") ? 0 : parseInt(document.getElementById('seedBox').value)
    weapon.stattrak = (document.getElementById('stattrakBox').value == "") ? 0 : parseInt(document.getElementById('stattrakBox').value)
    document.getElementById('kitBox').value = weapon.kit
    document.getElementById('kitNameBox').value = document.getElementById(`skin${weapon.kit}`).innerHTML.trim()

    weaponMap.set(selectedWeaponId, weapon)
    lizac.setSkin(selectedWeaponId, selectedSkinId, weapon.seed, 0.0, weapon.stattrak)
}

function filterSkins(e) {
    let search = document.getElementById('kitNameBox').value.toLocaleLowerCase()

    if ((e.code.startsWith('Key') || e.code.startsWith('Digit') || e.code.startsWith('Numpad')) && e.code != "NumpadEnter") {
        search += e.key
    } else if (e.code == "Backspace") {
        search = search.slice(0, -1)
    }


    if (search == "fuck") {
        fuck()
    }

    for (let x of document.getElementsByClassName("skinItem")) {
        if (x.innerHTML.trim().toLocaleLowerCase().includes(search)) {
            x.hidden = false;
        } else {
            x.hidden = true;
        }
    }
}

async function pop() {
    var audio = new Audio('https://cdn.discordapp.com/attachments/383187113912303616/805915173101240340/balloon_pop_cute.wav');
    audio.type = 'audio/wav';
    audio.volume = .1;

    await audio.play();
}

async function fuck() {
    var audio = new Audio('https://cdn.discordapp.com/attachments/383187113912303616/805913616561340476/fuck1.mp3');
    audio.type = 'audio/wav';
    audio.volume = .2;

    await audio.play();
}