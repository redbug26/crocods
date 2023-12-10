
var Module = {
    preRun: [],
    postRun: [
        function () {
            init_drag_and_drop();
            loadMediaInURL();
        },
    ],
    print: (function () {
        return function (text) {
            text = Array.prototype.slice.call(arguments).join(' ');
            console.log(text);
        };
    })(),
    printErr: function (text) {
        text = Array.prototype.slice.call(arguments).join(' ');
        console.error(text);
    },
    canvas: (function () {
        var canvas = document.getElementById('canvas');
        canvas.addEventListener("webglcontextlost", function (e) {alert('FIXME: WebGL context lost, please reload the page'); e.preventDefault();}, false);
        return canvas;
    })(),
    setStatus: function (text) { },
    monitorRunDependencies: function (left) { },
};
window.onerror = function (event) {
    console.log("onerror: " + event);
};
function id(id) {
    return document.getElementById(id);
}
function init_drag_and_drop() {
    id('canvas').addEventListener('dragenter', load_dragenter, false);
    id('canvas').addEventListener('dragleave', load_dragleave, false);
    id('canvas').addEventListener('dragover', load_dragover, false);
    id('canvas').addEventListener('drop', load_drop, false);
}
function load_dragenter(e) {
    e.stopPropagation();
    e.preventDefault();
}
function load_dragleave(e) {
    e.stopPropagation();
    e.preventDefault();
}
function load_dragover(e) {
    e.stopPropagation();
    e.preventDefault();
}
function load_drop(e) {
    e.stopPropagation();
    e.preventDefault();
    load_file(e.dataTransfer.files);
}
function load_file(files) {
    if (files.length > 0) {
        let file = files[0];
        if (file.size < (1024 * 1024)) {
            let reader = new FileReader();
            reader.onload = function (loadEvent) {
                console.log('file loaded!')
                let content = loadEvent.target.result;
                if (content) {
                    console.log('content length: ' + content.byteLength);
                    let uint8Array = new Uint8Array(content);
                    let res = Module['ccall']('emsc_load_data',
                        'int',
                        ['string', 'array', 'number'],  // name, data, size
                        [file.name, uint8Array, uint8Array.length]);
                    if (res == 0) {
                        console.warn('emsc_loadfile() failed!');
                    }
                }
                else {
                    console.warn('load result empty!');
                }
            };
            reader.readAsArrayBuffer(file);
        }
        else {
            console.warn('ignoring dropped file because it is too big')
        }
    }
}

var freeBottom = 0;

function do_resize() {

    var availWidth = Math.min(1024, document.getElementById("crocods-screen-fs-center").clientWidth);
    var scale = availWidth / VIRTUAL_KEYBOARD_WIDE_WIDTH;
    var keyboardHeight = 134 * scale;

    var width = window.innerWidth;
    var height = window.innerHeight - 35; // 42
    var ratio = 384.0 / 272.0;

    if ($("#crocods-virtual-keyboard").is(":visible")) {
        height = height - (keyboardHeight);
    }


    var dest_width = width;
    var dest_height = width / ratio;

    if (dest_height > height) {
        dest_width = height * ratio;
        dest_height = height;

        freeBottom = 0;
    } else {
        freeBottom = 1;
    }

    $('#canvas').css('width', dest_width);
    $('#canvas').css('height', dest_height);

    // ---

    scale = height / 228.0;
    if (scale > 2) {
        scale = 2;
    }

    var crocodsTouchLeft = document.getElementById('crocods-touch-left');
    crocodsTouchLeft.style.transform = "scale(" + scale.toFixed(8) + ")";
    crocodsTouchLeft.style.height = height + "px";

    var crocodsTouchRight = document.getElementById('crocods-touch-right');
    crocodsTouchRight.style.transform = "scale(" + scale.toFixed(8) + ")";
    crocodsTouchRight.style.height = height + "px";

    // ---

    show_doc();
}

var getUrlParameter = function getUrlParameter(sParam) {
    var sPageURL = window.location.search.substring(1),
        sURLVariables = sPageURL.split('&'),
        sParameterName,
        i;

    for (i = 0; i < sURLVariables.length; i++) {
        sParameterName = sURLVariables[i].split('=');

        if (sParameterName[0] === sParam) {
            return sParameterName[1] === undefined ? true : decodeURIComponent(sParameterName[1]);
        }
    }
};

function show_doc() {
    $('#doc').clearQueue();
    $('#doc').show();

    if (freeBottom == 0) {
        $('#doc').delay(5000).fadeOut(400);
    }
}

function onFileInputChange(e) {
    e.returnValue = false;  // IE
    e.preventDefault();
    e.stopPropagation();
    e.target.focus();
    if (!this.files || this.files.length === 0) return;           // this will have a property "files"!

    var file = this.files[0];

    // Tries to clear the last selected file so the same file can be chosen
    try {
        fileInputElement.value = "";
    } catch (ex) {
        // Ignore
    }

    var reader = new FileReader();
    reader.onload = function (event) {
        var uint8Array = new Uint8Array(event.target.result);


        let res = Module['ccall']('emsc_load_data',
            'int',
            ['string', 'array', 'number'],  // name, data, size
            [file.name, uint8Array, uint8Array.length]);
        if (res == 0) {
            console.warn('emsc_loadfile() failed!');
        }
    };
    reader.onerror = function (event) {
        alert("File reading error: " + event.target.error.name);
    };

    reader.readAsArrayBuffer(file);

    // if (files && files.length > 0) {
    //     if (files.length === 1)
    //         self.readFromFile(files[0], chooserOpenType, chooserPort, chooserAltPower, chooserAsExpansion, resume);
    //     else
    //         self.readFromFiles(files, chooserOpenType, chooserPort, chooserAltPower, chooserAsExpansion, resume);
    // }

    return false;
}

function createFileInputElement() {
    fileInputElement = document.createElement("input");
    fileInputElement.id = "crocods-file-loader-input";
    fileInputElement.type = "file";
    fileInputElement.multiple = true;
    fileInputElement.accept = ".dsk,.cpr,.bas,.zip";
    fileInputElement.style.display = "none";
    fileInputElement.addEventListener("change", onFileInputChange);
    document.getElementById("crocods-screen-fs-center").appendChild(fileInputElement);
}

$(window).resize(function () {
    do_resize();

    var fsElementCenter = document.getElementById("crocods-screen-fs-center");
    updateKeyboardWidth(fsElementCenter.clientWidth);
});

function loadMediaInURL() {
    var url = getUrlParameter('url');

    if (crococonfig.image) {
        url = crococonfig.image;
    }

    if (url != null) {
        var link = document.createElement("a");
        link.href = url;

        var oReq = new XMLHttpRequest();
        oReq.open("GET", link.href, true);
        oReq.responseType = "arraybuffer";

        oReq.onerror = function () {
            $('#dialogurl').attr("href", link.href);
            $("#dialog").dialog();
        };

        oReq.onload = function (oEvent) {

            if ((crococonfig.keyemul) && (crococonfig.keyemul == "joystick")) {
                execute_menu(12);       // Switch keyboard -> joystick emulation
            }


            var content = oReq.response;

            console.log('content length: ' + content.byteLength);
            let uint8Array = new Uint8Array(content);
            let res = Module['ccall']('emsc_load_data',
                'int',
                ['string', 'array', 'number'],  // name, data, size
                [url.name, uint8Array, uint8Array.length]);
            if (res == 0) {
                console.warn('emsc_loadfile() failed!');
            }
        };

        oReq.send();
    }
}

var VIRTUAL_KEYBOARD_WIDE_WIDTH = 494, VIRTUAL_KEYBOARD_NARROW_WIDTH = 419, VIRTUAL_KEYBOARD_HEIGHT = 161;


function updateKeyboardWidth(maxWidth) {
    var virtualKeyboardElement = document.getElementById('crocods-virtual-keyboard');
    var availWidth = Math.min(1024, maxWidth);       // Limit to 1024px
    var width = VIRTUAL_KEYBOARD_WIDE_WIDTH;
    var scale = availWidth / width;
    virtualKeyboardElement.style.width = "" + width + "px";
    virtualKeyboardElement.style.transform = "translateX(-50%) scale(" + scale.toFixed(8) + ")";

    // var crocodsTouchLeft = document.getElementById('crocods-touch-left');
    // crocodsTouchLeft.style.transform = "scale(" + scale.toFixed(8) + ")";

    // var crocodsTouchRight = document.getElementById('crocods-touch-right');
    // crocodsTouchRight.style.transform = "scale(" + scale.toFixed(8) + ")";

    return {w: availWidth, h: Math.ceil(VIRTUAL_KEYBOARD_HEIGHT * scale)};
}

function press_key(press, scancode) {
    Module['ccall']('emsc_press_key',
        'int',
        ['number', 'number'],  // pressed, key_number
        [press, scancode]);
}

function press_pad(press, buttonid) {
    Module['ccall']('emsc_press_pad',
        'int',
        ['number', 'number'],  // pressed, key_number
        [press, buttonid]);
}

function execute_menu(menuid) {
    Module['ccall']('emsc_execute_menu',
        'int',
        ['number'],  // menuid
        [menuid]);
}

function initKey(name, scancode) {
    $(name).on('mouseup mouseleave touchend', function (e) {
        e.preventDefault();

        // console.log("touch up " + name);
        $(name).removeClass("crocods-keyboard-keydown");
        press_key(0, scancode);
    }).on('mousedown touchstart', function (e) {
        e.preventDefault();

        // console.log("touch down " + name);
        $(name).addClass("crocods-keyboard-keydown");
        press_key(1, scancode);
        // if (window.navigator && window.navigator.vibrate) {
        //     window.navigator.vibrate(200);
        // }
    });
}

function initPad(name, scancode) {
    $(name).on('mouseup mouseleave touchend', function (e) {
        e.preventDefault();

        $(name).removeClass("crocods-pad-keydown");
        press_pad(0, scancode);
    }).on('mousedown touchstart', function (e) {
        e.preventDefault();

        $(name).addClass("crocods-pad-keydown");
        press_pad(1, scancode);
        // if (window.navigator && window.navigator.vibrate) {
        //     window.navigator.vibrate(200);
        // }
    });

}

function dispBrowser() {
    var oReq = new XMLHttpRequest();
    oReq.open("GET", "browse.php", true);
    oReq.responseType = "arraybuffer";

    oReq.onload = function (oEvent) {
        var arrayBuffer = oReq.response;

        var uint8Array = new Uint8Array(arrayBuffer);
        let res = Module['ccall']('emsc_load_browsebuffer',
            'int',
            ['array', 'number'],  // name, data, size
            [uint8Array, uint8Array.length]);
        if (res == 0) {
            console.warn('emsc_load_browsebuffer() failed!');
        }
    };

    oReq.send();
}

function loadURL(url, filename) {
    console.log("loadURL " + url);
    var oReq = new XMLHttpRequest();
    oReq.open("GET", url, true);
    oReq.responseType = "arraybuffer";

    oReq.onload = function (oEvent) {
        var arrayBuffer = oReq.response;

        var uint8Array = new Uint8Array(arrayBuffer);

        let res = Module['ccall']('emsc_load_data',
            'int',
            ['string', 'array', 'number'],  // name, data, size
            [filename, uint8Array, uint8Array.length]);
        if (res == 0) {
            console.warn('emsc_loadfile() failed!');
        }
    };

    oReq.send();
}

function loadCPCPOWER(cpcpowerid) {
    var url = "browse.php?action=cpcpower&id=" + cpcpowerid;

    console.log("loadURL " + url);
    var oReq = new XMLHttpRequest();
    oReq.open("GET", url, true);
    oReq.responseType = "arraybuffer";

    oReq.onload = function (oEvent) {
        var arrayBuffer = oReq.response;

        var uint8Array = new Uint8Array(arrayBuffer);

        let res = Module['ccall']('emsc_load_data',
            'int',
            ['string', 'array', 'number'],  // name, data, size
            [url.name, uint8Array, uint8Array.length]);
        if (res == 0) {
            console.warn('emsc_loadfile() failed!');
        }
    };

    oReq.send();

}


// Work-around chromium autoplay policy
// https://github.com/emscripten-core/emscripten/issues/6511
function resumeAudio(e) {
    if (typeof Module === 'undefined'
        || typeof Module.SDL2 == 'undefined'
        || typeof Module.SDL2.audioContext == 'undefined')
        return;
    if (Module.SDL2.audioContext.state == 'suspended') {
        Module.SDL2.audioContext.resume();
    }
    if (Module.SDL2.audioContext.state == 'running') {
        document.getElementById('canvas').removeEventListener('click', resumeAudio);
        document.removeEventListener('keydown', resumeAudio);
    }
}


function crocodsAppIsReady() {
    var cpcpowerid = getUrlParameter('cpcpowerid');
    if (cpcpowerid != null) {
        loadCPCPOWER(cpcpowerid);
    }

    var idx = window.location.href.lastIndexOf("cpcpower/");
    if (idx > 0) {
        var cpcpowerid = window.location.href.substr(idx + 9);
        loadCPCPOWER(cpcpowerid);
    }

    document.getElementById('canvas').addEventListener('click', resumeAudio);
    document.addEventListener('keydown', resumeAudio);
}

var fileInputElement;


$(document).ready(function () {
    do_resize();

    var fsElementCenter = document.getElementById("crocods-screen-fs-center");
    updateKeyboardWidth(fsElementCenter.clientWidth);

    $('#main').click(function () {
        // show_doc();  
    });

    $('#crocods-bar-switchkey').click(function () {
        menu_id = $('#crocods-bar-switchkey').data("menuid");

        execute_menu(menu_id);

        // if (menu_id == 10) {
        //     $('#crocods-bar-switchkey').css("background-position", "-237px -76px");
        //     $('#crocods-bar-switchkey').data("menuid", 12);
        // } else if (menu_id == 12) {
        //     $('#crocods-bar-switchkey').css("background-position", "-213px -76px");
        //     $('#crocods-bar-switchkey').data("menuid", 10);
        // }
    });

    $('#crocods-bar-fullscreen').click(function () {
        var elem = document.documentElement;

        /* View in fullscreen */
        if (elem.requestFullscreen) {
            elem.requestFullscreen();
        } else if (elem.webkitRequestFullscreen) { /* Safari */
            elem.webkitRequestFullscreen();
        } else if (elem.msRequestFullscreen) { /* IE11 */
            elem.msRequestFullscreen();
        }
    });


    $('#crocods-bar-logo').click(function () {
        window.location = "http://crocods.org";
    });

    $('#crocods-bar-gear').click(function () {
        execute_menu(36);
    });

    $('#crocods-bar-database').click(function () {
        execute_menu(74);
    });


    $('#crocods-bar-keyboard').click(function () {
        $("#crocods-virtual-keyboard").toggle();
        do_resize();
    });

    $('#crocods-bar-diska').click(function () {
        if (!fileInputElement) createFileInputElement();
        fileInputElement.click();
    });

    initKey('.crocods-keyboard-up', 0);     // CPC_CURSOR_UP,     // = 0
    initKey('.crocods-keyboard-right', 1);     // CPC_CURSOR_RIGHT,
    initKey('.crocods-keyboard-down', 2);    // CPC_CURSOR_DOWN,
    initKey('.crocods-keyboard-num_9', 3);    // CPC_F9,
    initKey('.crocods-keyboard-num_6', 4);    // CPC_F6,
    initKey('.crocods-keyboard-num_3', 5);    // CPC_F3,
    initKey('.crocods-keyboard-code', 6);// CPC_SMALL_ENTER,
    initKey('.crocods-keyboard-num_period', 7);// CPC_FDOT,
    // /* line 1, bit 0..bit 7 */
    initKey('.crocods-keyboard-left', 8);// CPC_CURSOR_LEFT,
    initKey('.crocods-keyboard-graph', 9);// CPC_COPY,
    initKey('.crocods-keyboard-num_7', 10);// CPC_F7,
    initKey('.crocods-keyboard-num_8', 11);   // CPC_F8,
    initKey('.crocods-keyboard-num_5', 12);    // CPC_F5,
    initKey('.crocods-keyboard-num_1', 13);    // CPC_F1,
    initKey('.crocods-keyboard-num_2', 14);    // CPC_F2,
    initKey('.crocods-keyboard-num_0', 15);    // CPC_F0,
    // /* line 2, bit 0..bit 7 */
    initKey('.crocods-keyboard-backslash', 16);    // CPC_CLR,
    initKey('.crocods-keyboard-close_bracket', 17);    // CPC_OPEN_SQUARE_BRACKET,
    initKey('.crocods-keyboard-enter', 18);    // CPC_RETURN,
    initKey('.crocods-keyboard-backquote', 19);    // CPC_CLOSE_SQUARE_BRACKET,
    initKey('.crocods-keyboard-num_4', 20);    // CPC_F4,
    initKey('.crocods-keyboard-shift', 21);    // CPC_SHIFT,
    initKey('.crocods-keyboard-shift2', 21);    // CPC_SHIFT,
    initKey('.crocods-keyboard-dead', 22);    // CPC_FORWARD_SLASH,
    initKey('.crocods-keyboard-capslock', 23);    // CPC_CONTROL,
    // /* line 3, bit 0.. bit 7 */
    initKey('.crocods-keyboard-equal', 24);    // CPC_HAT,
    initKey('.crocods-keyboard-minus', 25);    // CPC_MINUS,
    initKey('.crocods-keyboard-open_bracket', 26);    // CPC_AT,
    initKey('.crocods-keyboard-p', 27);    // CPC_P,
    initKey('.crocods-keyboard-quote', 28);    // CPC_SEMICOLON,
    initKey('.crocods-keyboard-semicolon', 29);    // CPC_COLON,
    initKey('.crocods-keyboard-slash', 30);    // CPC_BACKSLASH,
    initKey('.crocods-keyboard-period', 31);    // CPC_DOT,
    // /* line 4, bit 0..bit 7 */
    initKey('.crocods-keyboard-d0', 32);    // CPC_ZERO,
    initKey('.crocods-keyboard-d9', 33);    // CPC_9,
    initKey('.crocods-keyboard-o', 34);    // CPC_O,
    initKey('.crocods-keyboard-i', 35);    // CPC_I,
    initKey('.crocods-keyboard-l', 36);    // CPC_L,
    initKey('.crocods-keyboard-k', 37);    // CPC_K,
    initKey('.crocods-keyboard-m', 38);    // CPC_M,
    initKey('.crocods-keyboard-comma', 39);    // CPC_COMMA,
    // /* line 5, bit 0..bit 7 */
    initKey('.crocods-keyboard-d8', 40);    // CPC_8,
    initKey('.crocods-keyboard-d7', 41);    // CPC_7,
    initKey('.crocods-keyboard-u', 42);    // CPC_U,
    initKey('.crocods-keyboard-y', 43);    // CPC_Y,
    initKey('.crocods-keyboard-h', 44);    // CPC_H,
    initKey('.crocods-keyboard-j', 45);    // CPC_J,
    initKey('.crocods-keyboard-n', 46);    // CPC_N,
    initKey('.crocods-keyboard-space', 47);    // CPC_SPACE,
    // /* line 6, bit 0..bit 7 */
    initKey('.crocods-keyboard-d6', 48);    // CPC_6,
    initKey('.crocods-keyboard-d5', 49);    // CPC_5,
    initKey('.crocods-keyboard-r', 50);    // CPC_R,
    initKey('.crocods-keyboard-t', 51);    // CPC_T,
    initKey('.crocods-keyboard-g', 52);    // CPC_G,
    initKey('.crocods-keyboard-f', 53);    // CPC_F,
    initKey('.crocods-keyboard-b', 54);    // CPC_B,
    initKey('.crocods-keyboard-v', 55);    // CPC_V,
    // /* line 7, bit 0.. bit 7 */
    initKey('.crocods-keyboard-d4', 56);    // CPC_4,
    initKey('.crocods-keyboard-d3', 57);    // CPC_3,
    initKey('.crocods-keyboard-e', 58);    // CPC_E,
    initKey('.crocods-keyboard-w', 59);    // CPC_W,
    initKey('.crocods-keyboard-s', 60);    // CPC_S,
    initKey('.crocods-keyboard-d', 61);    // CPC_D,
    initKey('.crocods-keyboard-c', 62);    // CPC_C,
    initKey('.crocods-keyboard-x', 63);    // CPC_X,
    // /* line 8, bit 0.. bit 7 */
    initKey('.crocods-keyboard-d1', 64);    // CPC_1,
    initKey('.crocods-keyboard-d2', 65);    // CPC_2,
    initKey('.crocods-keyboard-escape', 66); // CPC_ESC,
    initKey('.crocods-keyboard-q', 67); // CPC_Q,
    initKey('.crocods-keyboard-tab', 68); // CPC_TAB,
    initKey('.crocods-keyboard-a', 69);  // CPC_A,
    initKey('.crocods-keyboard-control', 70);    // CPC_CAPS_LOCK,
    initKey('.crocods-keyboard-z', 71);    // CPC_Z,
    // /* line 9, bit 7..bit 0 */
    // initKey('.crocods-joystick-up', 72);    // CPC_JOY_UP,
    // initKey('.crocods-joystick-down', 73);    // CPC_JOY_DOWN,
    // initKey('.crocods-joystick-left', 74);    // CPC_JOY_LEFT,
    // initKey('.crocods-joystick-right', 75);    // CPC_JOY_RIGHT,
    // initKey('.crocods-touch-button-joy-A', 76);    // CPC_JOY_FIRE1,
    // initKey('.crocods-touch-button-joy-B', 77);    // CPC_JOY_FIRE2,
    initKey('.crocods-keyboard-', 78);    // CPC_SPARE,
    initKey('.crocods-keyboard-backspace', 79);    // CPC_DEL,



    initKey('#crocods-touch-T_F', 13);    // CPC_F1,
    initKey('#crocods-touch-T_D', 66); // CPC_ESC,
    initKey('#crocods-touch-T_E', 47);    // CPC_SPACE,
    initKey('#crocods-touch-T_G', 18); // CPC_RETURN,



    initPad('.crocods-touch-button-joy-A', 4);
    initPad('.crocods-touch-button-joy-B', 5);

    // /* SELECT BUTTON */
    // button |= button_state[11] ? (1 << 2) : 0;

    // /* START BUTTON */
    // button |= button_state[10] ? (1 << 3) : 0;

    initPad('.crocods-joystick-right', 1);
    initPad('.crocods-joystick-left', 0);
    initPad('.crocods-joystick-up', 2);
    initPad('.crocods-joystick-down', 3);

    // /* R1	*/
    // button |= button_state[9] ? (1 << 8) : 0;
    // /* L1	*/
    // button |= button_state[8] ? (1 << 9) : 0;

    // /* X    */
    // button |= button_state[6] ? (1 << 10) : 0;
    // /* Y    */
    // button |= button_state[7] ? (1 << 11) : 0;

    // /* R2    */
    // button |= button_state[14] ? (1 << 14) : 0;
    // /* L2    */
    // button |= button_state[13] ? (1 << 15) : 0;






});

