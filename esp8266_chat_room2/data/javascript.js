var username, PreambleLength = 0, SyncWord = 0;
openModal("myModal");

window.onload = function() {
    
}

document.getElementById("TxPower").oninput = function(){
    if (document.getElementById("PA_mode").value == "PA_OUTPUT_PA_BOOST_PIN"){
        if(document.getElementById("TxPower").value < 2){
            document.getElementById("TxPower").value = 2;
        }
        else if(document.getElementById("TxPower").value > 20){
            document.getElementById("TxPower").value = 20;
        }
    }else{
        if(document.getElementById("TxPower").value < 0){
            document.getElementById("TxPower").value = 0;
        }
        else if(document.getElementById("TxPower").value > 14){
            document.getElementById("TxPower").value = 14;
        }
    }
}

function hideOnClickOutside(temp) {
console.log("event start")
var element = document.getElementById(temp);
    const outsideClickListener = event => {
    console.log(event.target.id)
        if (!element.contains(event.target) && isVisible(element)) { // or use: event.target.closest(selector) === null \\(!element.contains(event.target) && isVisible(element))
            closeModal("menuModal");
            removeClickListener()
        }
    }

    const removeClickListener = () => {
        document.removeEventListener('onclick', outsideClickListener)
        console.log("event removed")
    }
    
    document.addEventListener('onclick', outsideClickListener)
    console.log("event added")
}
const isVisible = elem => !!elem && !!( elem.offsetWidth || elem.offsetHeight || elem.getClientRects().length )

function uploadModal() {
    closeModal("menuModal");
    openModal("uploadModal");
}

function settingsModal(){
    var text = "";
    var tempraryString = "";
    var settingindex = 0;
    var Pa=0,TxPower = 0, SignalBandwidth = 0.0, SpreadingFactor = 0, CodingRate4 = 0, Gain = 0;
    closeModal("menuModal");
    openModal("settingsModal");
    var shr = new XMLHttpRequest();
    shr.open("GET", "http://10.10.10.1/settingsToBrowser", true);
    shr.onload = function () {
        if (shr.status === 200) {
            text = shr.responseText;
            console.log(text);
            for (var i = 0; i < text.length; i++) {
                if (text.charAt(i) != ' '){
                    tempraryString += text.charAt(i);
                }
                else
                {
                  switch (settingindex)
                  {
                  case 0:
                    Pa = parseInt(tempraryString);
                  case 1:
                    TxPower = parseInt(tempraryString);
                    break;
                  case 2:
                    SignalBandwidth = parseFloat(tempraryString);
                    break;
                  case 3:
                    SpreadingFactor = parseInt(tempraryString);
                    break;
                  case 4:
                    CodingRate4 = parseInt(tempraryString);
                    break;
                  case 5:
                    PreambleLength = parseInt(tempraryString);
                    break;
                  case 6:
                    SyncWord = parseInt(Number(tempraryString), 10);
                    break;
                  case 7:
                    Gain = parseInt(tempraryString);
                    break;
                  default:
                    break;
                  }
                  settingindex++;
                  tempraryString = "";
                }
            }
            if(Pa == 0){
                document.getElementById("PA_mode").value = "PA_OUTPUT_PA_BOOST_PIN";
            }else{
                document.getElementById("PA_mode").value = "PA_OUTPUT_RFO_PIN";
            }
            document.getElementById("TxPower").value = TxPower;
            document.getElementById("SignalBandwidth").value = SignalBandwidth;
            document.getElementById("SpreadingFactor").value = SpreadingFactor;
            document.getElementById("CodingRate").value = CodingRate4;
            document.getElementById("Gain").value = Gain;
            
        }
        else {
        alert('Request failed.  Returned status of ' + shr.status);
        }
    };
    shr.send();
}

function settingSubmit(){
    var Pa=0,TxPower = 0, SignalBandwidth = 0.0, SpreadingFactor = 0, CodingRate4 = 0, Gain = 0;
    var Settingstoesp = "";
    if(document.getElementById("PA_mode").value == "PA_OUTPUT_PA_BOOST_PIN"){
        Pa = 0;
    }else{
        Pa = 1;
    }
    TxPower = document.getElementById("TxPower").value;
    SignalBandwidth = document.getElementById("SignalBandwidth").value;
    SpreadingFactor = document.getElementById("SpreadingFactor").value;
    CodingRate4 = document.getElementById("CodingRate").value;
    Gain = document.getElementById("Gain").value;
    Settingstoesp = Pa.toString() + " " + TxPower.toString() + " " + (SignalBandwidth/1000).toString() + "E3 " + SpreadingFactor.toString() + " " + CodingRate4.toString() + " " + PreambleLength.toString() + " 0x" + SyncWord.toString(16) + " " + Gain.toString() + " ";
    console.log(Settingstoesp);

    var shr = new XMLHttpRequest();
    shr.open('POST', 'http://10.10.10.1/settingsToesp');
    shr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    shr.onload = function () {
        if (shr.status === 200) {
        console.log(shr)
        }
        else if (shr.status !== 200) {
        alert('Request failed.  Returned status of ' + shr.status);
        }
    };
    shr.send(encodeURI("settingsToEsp=" + Settingstoesp));
    closeModal("settingsModal");
    alert("The ESP Reset in Progress... Please refresh the page.");
}

function appendToMessages(message) {
    var ul = document.getElementById("messages");
    var li = document.createElement("li");
    //li.appendChild(document.createTextNode(message));
    //console.log(li);
    li.innerHTML = message;
    //li.appendChild(document.getElementById("li").innerHTML = message);
    li.setAttribute("id", "element4");
    ul.appendChild(li);
}

function setName(name) {
    username = document.getElementById("userId").value;
    closeModal("myModal");
    startReadingMessages();
}

function startReadingMessages() {
    setInterval(function () { readMessages(); }, 1500);
    }

    function closeModal(modalId) {
    var modal = document.getElementById(modalId);
    modal.style.display = "none";
    }

    function openModal(modalId) {
    var modal = document.getElementById(modalId);
    modal.style.display = "block";
}

function sendMessage() {
    var messageBox = document.getElementById("m");
    var message = username + ": " + messageBox.value;
    xhr = new XMLHttpRequest();
    xhr.open('POST', 'http://10.10.10.1/sendMessage');
    xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    xhr.onload = function () {
        if (xhr.status === 200) {
        messageBox.value = '';
        readMessages();
        }
        else if (xhr.status !== 200) {
        alert('Request failed.  Returned status of ' + xhr.status);
        }
    };
    xhr.send(encodeURI('message=' + message));
}

function htmlDecode(input){
    var e = document.createElement('div');
    e.innerHTML = input;
    return e.childNodes[0].nodeValue;
}

function readMessages() {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', 'http://10.10.10.1/readMessages');
    xhr.onload = function () {
        if (xhr.status === 200) {
        document.getElementById("messages").innerHTML = '';
        if (xhr.responseText.startsWith("&lt;a")){
            var msgArr = htmlDecode(xhr.responseText);
        }
        else{
            var msgArr = xhr.responseText.toString().replace(/\n$/, "").split(/\n/);
        }
        msgArr.forEach(function (entry) {
            appendToMessages(entry);
        });
        }
        else {
        alert('Request failed.  Returned status of ' + xhr.status);
        }
    };
    xhr.send();
}

function clearMessages() {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', 'http://10.10.10.1/clearMessages');
    xhr.onload = function () {
        if (xhr.status === 200) {
        closeModal("menuModal");
        }
        else {
        alert('Request failed.  Returned status of ' + xhr.status);
        }
    };
    xhr.send();
}