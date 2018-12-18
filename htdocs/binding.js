var afbVshlSupport;
var ws;
var evtIdx = 0;
var count = 0;

//**********************************************
// Logger
//**********************************************
var log = {
  command: function (url, api, verb, query) {
    console.log("subscribe api=" + api + " verb=" + verb + " query=", query);
    var question = url + "/" + api + "/" + verb + "?query=" + JSON.stringify(query);
    log._write("question", count + ": " + log.syntaxHighlight(question));
  },

  event: function (obj) {
    console.log("gotevent:" + JSON.stringify(obj));
    log._write("outevt", (evtIdx++) + ": " + JSON.stringify(obj));
  },

  reply: function (obj) {
    console.log("replyok:" + JSON.stringify(obj));
    log._write("output", count + ": OK: " + log.syntaxHighlight(obj));
  },

  error: function (obj) {
    console.log("replyerr:" + JSON.stringify(obj));
    log._write("output", count + ": ERROR: " + log.syntaxHighlight(obj));
  },

  _write: function (element, msg) {
    var el = document.getElementById(element);
    el.innerHTML += msg + '\n';

    // auto scroll down
    setTimeout(function () {
      el.scrollTop = el.scrollHeight;
    }, 100);
  },

  syntaxHighlight: function (json) {
    if (typeof json !== 'string') {
      json = JSON.stringify(json, undefined, 2);
    }
    json = json.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
    return json.replace(/("(\\u[a-zA-Z0-9]{4}|\\[^u]|[^\\"])*"(\s*:)?|\b(true|false|null)\b|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?)/g, function (match) {
      var cls = 'number';
      if (/^"/.test(match)) {
        if (/:$/.test(match)) {
          cls = 'key';
        } else {
          cls = 'string';
        }
      } else if (/true|false/.test(match)) {
        cls = 'boolean';
      } else if (/null/.test(match)) {
        cls = 'null';
      }
      return '<span class="' + cls + '">' + match + '</span>';
    });
  },
};

//**********************************************
// Generic function to call binder
//***********************************************
function callbinder(url, api, verb, query) {
  log.command(url, api, verb, query);

  // ws.call return a Promise
  return ws.call(api + '/' + verb, query)
    .then(function (res) {
      log.reply(res);
      count++;
      return res;
    })
    .catch(function (err) {
      log.reply(err);
      count++;
      throw err;
    });
};


//**********************************************
// connect - establish Websocket connection
//**********************************************

function connect(elemID, api, verb, query) {
//  connectVshl(elemID, api, verb, query);
  connectVshlSupport(elemID, api, verb, query);
}
var newCallId;
function onCapabilityEvent(eventDataObj) {
    if (eventDataObj.event == "vshlsupport/dial") {
        newCallId = JSON.parse(eventDataObj.data).callId;
        console.log("New Dial Directive received. Let's send ringing state back");
        // send ringing state back.
        var query = {
            "action": "call_state_changed",
            "payload": {
                 "callId": newCallId,
                 "state":"OUTBOUND_RINGING"
            }
        }
        callbinder(afbVshlSupport.url, 'vshlsupport', 'phonecontrol/publish', query);
    }
}

function triggerCallStateChangedAction(newCallstate) {
    // send ringing state back.
    var query = {
        "action": "call_state_changed",
        "payload": {
             "callId": newCallId,
             "state": newCallstate
        }
    }
    callbinder(afbVshlSupport.url, 'vshlsupport', 'phonecontrol/publish', query);
}

function connectVshlSupport(elemID, api, verb, query) {

  function onopen() {
    document.getElementById("main").style.visibility = "visible";
    document.getElementById("connected").innerHTML = "VSHL Binder WS Active";
    document.getElementById("connected").style.background = "lightgreen";
    ws.onevent("*", onCapabilityEvent);
  }

  function onabort() {
    document.getElementById("main").style.visibility = "hidden";
    document.getElementById("connected").innerHTML = "Connected Closed";
    document.getElementById("connected").style.background = "red";
  }

  var urlparams = {
    base: "api",
    token: "HELLO",
  };
  const vshlSupportAddressInput = document.getElementById('vshl-support-address');
  urlparams.host = vshlSupportAddressInput.value;

  afbVshlSupport = new AFB(urlparams, "HELLO");
  ws = new afbVshlSupport.ws(onopen, onabort);
}

function clearPre(preId) {
  const pre = document.getElementById(preId);
  while (pre && pre.firstChild) {
    pre.removeChild(pre.firstChild);
  }
}

function showTemplateUIEventChooserDialog() {
  const modal = document.getElementById('templateui-event-chooser');
  const subscribeBtn = document.getElementById('templateui-subscribe-btn');

  subscribeBtn.addEventListener('click', (evt) => {
    const renderTemplate = document.getElementById('render_template').checked;
    const clearTemplate = document.getElementById('clear_template').checked;
    const renderPlayerInfo = document.getElementById('render_player_info').checked;
    const clearPlayerInfo = document.getElementById('clear_player_info').checked;

    const query = {"actions":[]};

    if (renderTemplate)
      query.actions.push('render_template');
    if (clearTemplate)
      query.actions.push('clear_template');
    if (renderPlayerInfo)
      query.actions.push('render_player_info');
    if (clearPlayerInfo)
      query.actions.push('clear_player_info');

    callbinder(afbVshlSupport.url, 'vshlsupport', 'guiMetadata/subscribe', query);
    modal.close();
  });

  // makes modal appear (adds `open` attribute)
  modal.showModal();
}

function showPhoneControlEventChooserDialog() {
  const modal = document.getElementById('phonecontrol-event-chooser');
  const subscribeBtn = document.getElementById('phonecontrol-subscribe-btn');

  subscribeBtn.addEventListener('click', (evt) => {
    const dial = document.getElementById('phonecontrol-dial').checked;
    const redial = document.getElementById('phonecontrol-redial').checked;
    const answer = document.getElementById('phonecontrol-answer').checked;
    const stop = document.getElementById('phonecontrol-stop').checked;
    const sendDtmf = document.getElementById('phonecontrol-send_dtmf').checked;

    const query = {"actions":[]};

    if (dial)
      query.actions.push('dial');
    if (redial)
      query.actions.push('redial');
    if (answer)
      query.actions.push('answer');
    if (stop)
      query.actions.push('stop');
    if (sendDtmf)
      query.actions.push('send_dtmf');

    callbinder(afbVshlSupport.url, 'vshlsupport', 'phonecontrol/subscribe', query);
    modal.close();
  });

  // makes modal appear (adds `open` attribute)
  modal.showModal();
}

function showNavigationEventCHooserDialod() {
  const modal = document.getElementById('navigation-event-chooser');
  const subscribeBtn = document.getElementById('navigation-subscribe-btn');

  subscribeBtn.addEventListener('click', (evt) => {
    const setDestination = document.getElementById('set_destination').checked;
    const cancelNavigation = document.getElementById('cancel_navigation').checked;

    const query = {"actions":[]};

    if (setDestination)
      query.actions.push('set_destination');
    if (cancelNavigation)
      query.actions.push('cancel_navigation');

    callbinder(afbVshlSupport.url, 'vshlsupport', 'navigation/subscribe', query);
    modal.close();
  });

  // makes modal appear (adds `open` attribute)
  modal.showModal();
}

function triggerButtonPressedAction(button) {
    var paramsJson = {
        "action" : "button_pressed",
        "payload": {
            "button" : button
        }
    }
    callbinder(afbVshlSupport.url, 'vshlsupport', 'playbackcontroller/publish', paramsJson);     
}