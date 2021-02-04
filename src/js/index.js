const lizac = require("../../build/Release/lizac.node")
const weaponMap = require("../js/weapons")
const skinMap = require("../js/skins")
const fs = require("fs-extra")
lizac.initialize()

document.title = "Lizac v0.9 (beta)"

let triggerBinding = false
let assistBinding = false
let binds = { trigger: {} }

let openPanel = panel => {
    for (let x of document.getElementsByClassName("menupanel")) {
        if (x.id == panel) {
            x.hidden = false
        } else {
            x.hidden = true
        }
    }
}

window.addEventListener('DOMContentLoaded', () => {
    loadConfig()

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

    document.getElementById('glowTypeBox').addEventListener('change', (event) => {
        lizac.toggleGlowColorMode(event.currentTarget.checked)
        if (event.currentTarget.checked) {
            document.getElementById('glowTypeBoxText').innerHTML = "Glow by Enemy"
            document.getElementById('tColorText').innerHTML = "Enemy Color"
            document.getElementById('ctColorText').innerHTML = "Team Color"
        } else {
            document.getElementById('glowTypeBoxText').innerHTML = "Glow by Team"
            document.getElementById('tColorText').innerHTML = "T Color"
            document.getElementById('ctColorText').innerHTML = "CT Color"
        }
    })

    lizac.toggleGlowColorMode(document.getElementById('glowTypeBox').checked)

    document.getElementById('skinBox').addEventListener('change', (event) => {
        lizac.toggleSkins(event.currentTarget.checked)
    })

    lizac.toggleSkins(document.getElementById('skinBox').checked)

    document.getElementById('ragdollBox').addEventListener('change', (event) => {
        if (document.getElementById('ragdollBox').checked) {
            lizac.setCvar("cl_ragdoll_gravity", parseInt(document.getElementById("ragdollAmmountbox").value))
        } else {
            lizac.setCvar("cl_ragdoll_gravity", 600)
        }
    })

    document.getElementById('ragdollAmmountbox').addEventListener('change', (event) => {
        lizac.setCvar("cl_ragdoll_gravity", parseInt(document.getElementById("ragdollAmmountbox").value))
    })

    if (document.getElementById('ragdollBox').checked) {
        lizac.setCvar("cl_ragdoll_gravity", parseInt(document.getElementById("ragdollAmmountbox").value))
    } else {
        lizac.setCvar("cl_ragdoll_gravity", 600)
    }
    document.getElementById('grenadePreviewBox').addEventListener('change', (event) => {
        console.log(document.getElementById('grenadePreviewBox').checked ? 1 : 0)
        lizac.setCvar("cl_grenadepreview", document.getElementById('grenadePreviewBox').checked ? 1 : 0)
    })

    if (document.getElementById('grenadePreviewBox').checked) {
        lizac.setCvar("cl_grenadepreview", document.getElementById('grenadePreviewBox').checked ? 1 : 0)
    }

    document.getElementById('ragdollTimeBox').addEventListener('change', (event) => {
        if (document.getElementById('ragdollTimeBox').checked) {
            lizac.setCvar("cl_phys_timescale", parseFloat(document.getElementById("ragdollTimeAmmountbox").value))
        } else {
            lizac.setCvar("cl_phys_timescale", 1)
        }
    })

    document.getElementById('ragdollTimeAmmountbox').addEventListener('change', (event) => {
        lizac.setCvar("cl_phys_timescale", parseFloat(document.getElementById("ragdollTimeAmmountbox").value))
    })

    if (document.getElementById('ragdollTimeBox').checked) {
        lizac.setCvar("cl_phys_timescale", parseFloat(document.getElementById("ragdollTimeAmmountbox").value))
    } else {
        lizac.setCvar("cl_phys_timescale", 1)
    }

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
            lizac.setTriggerBind(e.code)
            triggerBinding = false
        }
    })

    document.addEventListener("mousedown", e => {
        if (triggerBinding) {
            let keyInfo = getvKey(e)
            binds.trigger.key = keyInfo.vKey
            binds.trigger.name = `Mouse${keyInfo.name}`
            document.getElementById("triggerBind").innerHTML = `Mouse${keyInfo.name}`
            lizac.setTriggerBind(binds.trigger.key)
            triggerBinding = false
        }
    })

})

let getvKey = e => {
    let vKey, name;
    switch (e.button) {
        case 0:
            vKey = 0x01
            name = 1
            break
        case 1:
            vKey = 0x04
            name = 3
            break
        case 2:
            vKey = 0x02
            name = 2
            break
        case 3:
            vKey = 0x05
            name = 5
            break
        case 4:
            vKey = 0x06
            name = 4
            break
    }
    return { vKey, name }

}

function hexToRgb(hex) {
    var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
    return result ? {
        r: parseInt(result[1], 16),
        g: parseInt(result[2], 16),
        b: parseInt(result[3], 16)
    } : null
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
                clearTimeout(tagTimeout - 1)
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
                clearTimeout(tagTimeout - 1)
            }
        }
        tag()
    }
}

function reverse(s) {
    return [...s].reverse().join("");
}

function setClanTag(tag) {
    let append = document.getElementById('nlTagBox').checked ? " \n" : ""
    if (document.getElementById('backwardsTagBox').checked) {
        lizac.setClanTag("\u202E" + reverse(append + tag))
        document.title = ("\u202E" + reverse(tag)) || "Lizac v0.9"
    } else {
        lizac.setClanTag(tag + append)
        document.title = tag || "Lizac v0.9"
    }
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

let saveSkins = () => {
    let skinSaveArray = []
    weaponMap.forEach((v, k) => {
        if (v.kit) {
            skinSaveArray.push({ id: k, skin: v })
        }
    })

    return skinSaveArray
}

let loadSkins = skinSaveArray => {
    skinSaveArray.forEach((v, k) => {
        weaponMap.set(v.id, v.skin)
    })
}

let loadBinds = bindSave => {
    document.getElementById("triggerBind").innerHTML = `${bindSave.trigger.name}`
    binds.trigger.key = bindSave.trigger.key
    lizac.setTriggerBind(binds.trigger.key)
}

let saveConfig = () => {
    let saveObject = {}
    saveObject.bhop = document.getElementById('bhopBox').checked
    saveObject.autostrafe = document.getElementById('autostrafeBox').checked
    saveObject.radar = document.getElementById('radarBox').checked
    saveObject.noFlash = document.getElementById('noflashBox').checked
    saveObject.trigger = document.getElementById('triggerBox').checked
    saveObject.recoil = document.getElementById('rcsBox').checked
    saveObject.glow = document.getElementById('glowBox').checked
    saveObject.staticTag = document.getElementById('staticTagBox').checked
    saveObject.scrollTag = document.getElementById('scrollTagBox').checked
    saveObject.buildTag = document.getElementById('buildTagBox').checked
    saveObject.nlTag = document.getElementById('nlTagBox').checked
    saveObject.backwardsTag = document.getElementById('backwardsTagBox').checked
    saveObject.skinChanger = document.getElementById('skinBox').checked
    saveObject.ragdoll = document.getElementById('ragdollBox').checked
    saveObject.ragdollAmmount = document.getElementById('ragdollAmmountbox').value
    saveObject.ragdollTime = document.getElementById('ragdollTimeBox').checked
    saveObject.ragdollTimeAmmount = document.getElementById('ragdollTimeAmmountbox').value
    saveObject.grenadePreview = document.getElementById('grenadePreviewBox').checked

    saveObject.tColor = document.getElementById('tColor').value
    saveObject.ctColor = document.getElementById('ctColor').value
    saveObject.tag = document.getElementById('tagbox').value
    saveObject.tagInterval = document.getElementById('tagintervalbox').value
    saveObject.skins = saveSkins()
    saveObject.binds = binds

    fs.outputJson("config.json", saveObject)
}

let loadConfig = () => {
    if (fs.pathExistsSync('config.json')) {
        let saveObject = fs.readJsonSync('config.json')
        document.getElementById('bhopBox').checked = saveObject.bhop
        document.getElementById('autostrafeBox').checked = saveObject.autostrafe
        document.getElementById('radarBox').checked = saveObject.radar
        document.getElementById('noflashBox').checked = saveObject.noFlash
        document.getElementById('triggerBox').checked = saveObject.trigger
        document.getElementById('rcsBox').checked = saveObject.recoil
        document.getElementById('glowBox').checked = saveObject.glow
        document.getElementById('staticTagBox').checked = saveObject.staticTag
        document.getElementById('scrollTagBox').checked = saveObject.scrollTag
        document.getElementById('buildTagBox').checked = saveObject.buildTag
        document.getElementById('nlTagBox').checked = saveObject.nlTag
        document.getElementById('backwardsTagBox').checked = saveObject.backwardsTag
        document.getElementById('skinBox').checked = saveObject.skinChanger
        document.getElementById('ragdollBox').checked = saveObject.ragdoll
        document.getElementById('ragdollAmmountbox').value = saveObject.ragdollAmmount
        document.getElementById('ragdollTimeBox').checked = saveObject.ragdollTime
        document.getElementById('ragdollTimeAmmountbox').value = saveObject.ragdollTimeAmmount
        document.getElementById('grenadePreviewBox').checked = saveObject.grenadePreview

        document.getElementById('tColor').value = saveObject.tColor
        document.getElementById('ctColor').value = saveObject.ctColor
        document.getElementById('tagbox').value = saveObject.tag
        document.getElementById('tagintervalbox').value = saveObject.tagInterval
            //loadBinds(saveObject.binds)
        loadSkins(saveObject.skins)
    } else {
        let saveObject = fs.readJsonSync('config_default.json')
        document.getElementById('bhopBox').checked = saveObject.bhop
        document.getElementById('autostrafeBox').checked = saveObject.autostrafe
        document.getElementById('radarBox').checked = saveObject.radar
        document.getElementById('noflashBox').checked = saveObject.noFlash
        document.getElementById('triggerBox').checked = saveObject.trigger
        document.getElementById('rcsBox').checked = saveObject.recoil
        document.getElementById('glowBox').checked = saveObject.glow
        document.getElementById('staticTagBox').checked = saveObject.staticTag
        document.getElementById('scrollTagBox').checked = saveObject.scrollTag
        document.getElementById('buildTagBox').checked = saveObject.buildTag
        document.getElementById('nlTagBox').checked = saveObject.nlTag
        document.getElementById('backwardsTagBox').checked = saveObject.backwardsTag
        document.getElementById('skinBox').checked = saveObject.skinChanger
        document.getElementById('ragdollBox').checked = saveObject.ragdoll
        document.getElementById('ragdollAmmountbox').value = saveObject.ragdollAmmount
        document.getElementById('ragdollTimeBox').checked = saveObject.ragdollTime
        document.getElementById('ragdollTimeAmmountbox').value = saveObject.ragdollTimeAmmount
        document.getElementById('grenadePreviewBox').checked = saveObject.grenadePreview

        document.getElementById('tColor').value = saveObject.tColor
        document.getElementById('ctColor').value = saveObject.ctColor
        document.getElementById('tagbox').value = saveObject.tag
        document.getElementById('tagintervalbox').value = saveObject.tagInterval
            //loadBinds(saveObject.binds)
        loadSkins(saveObject.skins)
    }
}

async function pop() {
    var audio = new Audio('https://cdn.discordapp.com/attachments/383187113912303616/805915173101240340/balloon_pop_cute.wav')
    audio.type = 'audio/wav'
    audio.volume = .1

    await audio.play()
}

async function fuck() {
    var audio = new Audio('https://cdn.discordapp.com/attachments/383187113912303616/805913616561340476/fuck1.mp3')
    audio.type = 'audio/wav'
    audio.volume = .2

    await audio.play()
}