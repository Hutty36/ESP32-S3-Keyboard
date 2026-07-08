#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "USB.h"
#include "USBHIDKeyboard.h"
#include "USBHIDMouse.h"
#include <Adafruit_NeoPixel.h>

#define LED_PIN 48

Adafruit_NeoPixel led(1, 48, NEO_GRB + NEO_KHZ800);

void setLED(uint8_t r, uint8_t g, uint8_t b) {
    led.setPixelColor(0, led.Color(r, g, b));
    led.setBrightness(50);
    led.show();
}

// WiFi Access Point
const char* ssid = "ESP32-S3-Keyboard";
const char* password = "12345678";

WebServer server(80);
USBHIDKeyboard Keyboard;
USBHIDMouse Mouse;

// Forward declarations
void pressModifierKey(String key);
void executeScript(String script);

// ---------- Modern Web Page ----------
String html() {
    return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 HID Keyboard & Mouse</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            min-height: 100vh;
            padding: 20px;
        }
        
        .container {
            max-width: 1000px;
            margin: 0 auto;
        }
        
        h1 {
            text-align: center;
            margin: 30px 0;
            font-size: 2.5em;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        
        h2 {
            margin-bottom: 15px;
        }
        
        h3 {
            margin: 15px 0 10px 0;
        }
        
        .card {
            background: rgba(255, 255, 255, 0.1);
            backdrop-filter: blur(10px);
            border-radius: 20px;
            padding: 30px;
            margin: 20px 0;
            box-shadow: 0 8px 32px rgba(0,0,0,0.3);
            border: 1px solid rgba(255,255,255,0.2);
        }
        
        textarea {
            width: 100%;
            height: 100px;
            font-size: 16px;
            padding: 15px;
            border: none;
            border-radius: 10px;
            background: rgba(255,255,255,0.9);
            color: #333;
            resize: vertical;
            margin: 10px 0;
            font-family: 'Courier New', monospace;
        }
        
        .script-textarea {
            height: 200px;
            background: rgba(0,0,0,0.3);
            color: #00ff00;
            border: 1px solid rgba(0,255,0,0.3);
        }
        
        .keyboard {
            display: flex;
            flex-direction: column;
            gap: 5px;
            margin: 10px 0;
        }
        
        .key-row {
            display: flex;
            gap: 3px;
            justify-content: center;
            flex-wrap: wrap;
        }
        
        .key {
            background: rgba(255,255,255,0.2);
            border: 1px solid rgba(255,255,255,0.3);
            color: white;
            padding: 8px 10px;
            border-radius: 8px;
            cursor: pointer;
            font-size: 13px;
            transition: all 0.2s;
            min-width: 38px;
            text-align: center;
            user-select: none;
            white-space: nowrap;
        }
        
        .key:hover {
            background: rgba(255,255,255,0.4);
            transform: translateY(-2px);
        }
        
        .key:active {
            background: rgba(100,200,255,0.6);
            transform: translateY(0);
        }
        
        .key.wide {
            min-width: 55px;
        }
        
        .key.extra-wide {
            min-width: 75px;
        }
        
        .key.special {
            background: rgba(100,150,255,0.3);
            font-size: 11px;
        }
        
        .key.fkey {
            background: rgba(150,100,255,0.3);
            font-size: 12px;
        }
        
        .key.navkey {
            background: rgba(100,200,150,0.3);
        }
        
        .key.arrowkey {
            background: rgba(255,150,100,0.3);
            font-size: 16px;
            min-width: 45px;
        }
        
        .modifier-row {
            display: flex;
            gap: 5px;
            margin: 15px 0;
            flex-wrap: wrap;
            justify-content: center;
        }
        
        .modifier-btn {
            background: rgba(255,255,255,0.2);
            border: 1px solid rgba(255,255,255,0.3);
            color: white;
            padding: 10px 15px;
            border-radius: 8px;
            cursor: pointer;
            font-size: 14px;
            transition: all 0.2s;
        }
        
        .modifier-btn.active {
            background: rgba(0,255,0,0.5);
            border-color: #00ff00;
        }
        
        .modifier-btn.altgr-active {
            background: rgba(255,165,0,0.5);
            border-color: #ffa500;
        }
        
        .modifier-btn:hover {
            background: rgba(255,255,255,0.3);
        }
        
        .layout-toggle {
            display: flex;
            gap: 10px;
            justify-content: center;
            margin: 15px 0;
        }
        
        .layout-btn {
            background: rgba(255,255,255,0.2);
            border: 1px solid rgba(255,255,255,0.3);
            color: white;
            padding: 10px 20px;
            border-radius: 8px;
            cursor: pointer;
            font-size: 16px;
            transition: all 0.2s;
        }
        
        .layout-btn.active-layout {
            background: rgba(0,255,0,0.5);
            border-color: #00ff00;
        }
        
        .layout-btn:hover {
            background: rgba(255,255,255,0.3);
        }
        
        .mouse-pad {
            width: 200px;
            height: 200px;
            background: rgba(255,255,255,0.2);
            border-radius: 20px;
            margin: 20px auto;
            position: relative;
            touch-action: none;
            cursor: crosshair;
        }
        
        .mouse-controls {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 10px;
            max-width: 300px;
            margin: 20px auto;
        }
        
        .preset-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
            gap: 10px;
            margin: 20px 0;
        }
        
        .preset-btn {
            background: rgba(255,255,255,0.2);
            border: 1px solid rgba(255,255,255,0.3);
            color: white;
            padding: 12px;
            border-radius: 10px;
            cursor: pointer;
            font-size: 14px;
            transition: all 0.3s ease;
        }
        
        .preset-btn:hover {
            background: rgba(255,255,255,0.3);
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(0,0,0,0.3);
        }
        
        .send-btn {
            background: linear-gradient(45deg, #f093fb, #f5576c);
            border: none;
            color: white;
            padding: 15px 40px;
            border-radius: 25px;
            cursor: pointer;
            font-size: 18px;
            font-weight: bold;
            transition: all 0.3s ease;
            margin: 20px auto;
            display: block;
        }
        
        .send-btn:hover {
            transform: scale(1.05);
            box-shadow: 0 10px 25px rgba(0,0,0,0.3);
        }
        
        .send-btn.green {
            background: linear-gradient(45deg, #4CAF50, #45a049);
        }
        
        .status {
            text-align: center;
            margin: 20px 0;
            padding: 10px;
            border-radius: 10px;
            background: rgba(0,255,0,0.2);
            display: none;
        }
        
        .combo-display {
            background: rgba(0,0,0,0.3);
            padding: 10px;
            border-radius: 8px;
            text-align: center;
            min-height: 40px;
            margin: 10px 0;
        }
        
        .nav-arrows {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 5px;
            max-width: 200px;
            margin: 10px auto;
        }
        
        .command-list {
            background: rgba(0,0,0,0.3);
            border-radius: 10px;
            padding: 15px;
            margin-top: 15px;
            max-height: 400px;
            overflow-y: auto;
            display: none;
        }
        
        .command-list.visible {
            display: block;
        }
        
        .command-item {
            padding: 5px;
            margin: 3px 0;
            border-bottom: 1px solid rgba(255,255,255,0.1);
            font-family: 'Courier New', monospace;
            font-size: 13px;
        }
        
        .command-category {
            color: #ffa500;
            font-weight: bold;
            margin-top: 10px;
            font-size: 14px;
        }
        
        .toggle-commands-btn {
            background: rgba(255,255,255,0.2);
            border: 1px solid rgba(255,255,255,0.3);
            color: white;
            padding: 10px 20px;
            border-radius: 8px;
            cursor: pointer;
            font-size: 14px;
            transition: all 0.2s;
            display: block;
            margin: 10px auto;
        }
        
        .toggle-commands-btn:hover {
            background: rgba(255,255,255,0.3);
        }
        
        @media (max-width: 600px) {
            h1 { font-size: 1.8em; }
            .key { padding: 6px 5px; font-size: 11px; min-width: 28px; }
            .keyboard { gap: 3px; }
            .key-row { gap: 2px; }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>⌨️🖱️ ESP32-S3 HID Controller</h1>
        
        <!-- Status Display -->
        <div id="status" class="status"></div>
        
        <!-- Script Input -->
        <div class="card">
            <h2>📜 HID Script</h2>
            <p style="font-size: 14px; margin-bottom: 10px;">Enter commands line by line. Each line executes one action.</p>
            <textarea id="scriptArea" class="script-textarea" placeholder="TYPE:Hello World
ENTER
DELAY:500
TYPE:This is automated!
ENTER
MOUSE:10,20
CLICK:LEFT
COMBO:CTRL+C
WAIT:1000
TYPE:Pasted!"></textarea>
            <div style="text-align: center;">
                <button class="send-btn green" onclick="executeScript()">▶️ Execute Script</button>
                <button class="toggle-commands-btn" onclick="toggleCommands()">📖 Show/Hide Commands</button>
            </div>
            
            <!-- Command Reference -->
            <div id="commandList" class="command-list">
                <div class="command-category">📝 Text & Typing:</div>
                <div class="command-item">TYPE:text - Type the specified text</div>
                <div class="command-item">KEY:a - Press a single character key</div>
                <div class="command-item">STRING:text - Alternative to TYPE</div>
                
                <div class="command-category">⌨️ Special Keys:</div>
                <div class="command-item">ENTER - Press Enter</div>
                <div class="command-item">TAB - Press Tab</div>
                <div class="command-item">SPACE - Press Space</div>
                <div class="command-item">BACKSPACE - Press Backspace</div>
                <div class="command-item">DELETE - Press Delete</div>
                <div class="command-item">ESCAPE/ESC - Press Escape</div>
                <div class="command-item">CAPSLOCK - Toggle Caps Lock</div>
                <div class="command-item">MENU - Context menu (Shift+F10)</div>
                
                <div class="command-category">🔧 Function Keys:</div>
                <div class="command-item">F1 to F12 - Press function key</div>
                
                <div class="command-category">🧭 Navigation:</div>
                <div class="command-item">HOME - Go to Home</div>
                <div class="command-item">END - Go to End</div>
                <div class="command-item">PAGEUP/PGUP - Page Up</div>
                <div class="command-item">PAGEDOWN/PGDN - Page Down</div>
                <div class="command-item">INSERT/INS - Insert</div>
                <div class="command-item">PRINTSCREEN/PRTSC - Print Screen</div>
                <div class="command-item">SCROLLLOCK - Scroll Lock</div>
                <div class="command-item">PAUSE - Pause/Break</div>
                
                <div class="command-category">⬆️ Arrow Keys:</div>
                <div class="command-item">UP - Arrow Up</div>
                <div class="command-item">DOWN - Arrow Down</div>
                <div class="command-item">LEFT - Arrow Left</div>
                <div class="command-item">RIGHT - Arrow Right</div>
                
                <div class="command-category">🎯 Modifier Keys:</div>
                <div class="command-item">CTRL - Left Control</div>
                <div class="command-item">SHIFT - Left Shift</div>
                <div class="command-item">ALT - Left Alt</div>
                <div class="command-item">GUI/WIN/CMD - Windows/Command key</div>
                <div class="command-item">ALTGR - Right Alt (AltGr)</div>
                
                <div class="command-category">⚡ Key Combinations:</div>
                <div class="command-item">COMBO:CTRL+C - Copy</div>
                <div class="command-item">COMBO:CTRL+V - Paste</div>
                <div class="command-item">COMBO:CTRL+X - Cut</div>
                <div class="command-item">COMBO:CTRL+Z - Undo</div>
                <div class="command-item">COMBO:CTRL+A - Select All</div>
                <div class="command-item">COMBO:CTRL+S - Save</div>
                <div class="command-item">COMBO:ALT+TAB - Switch windows</div>
                <div class="command-item">COMBO:CTRL+ALT+DEL - Secure attention</div>
                <div class="command-item">COMBO:WIN+R - Run dialog</div>
                <div class="command-item">COMBO:WIN+E - File Explorer</div>
                
                <div class="command-category">🖱️ Mouse Control:</div>
                <div class="command-item">MOUSE:x,y - Move mouse by x,y pixels</div>
                <div class="command-item">CLICK:LEFT - Left mouse click</div>
                <div class="command-item">CLICK:RIGHT - Right mouse click</div>
                <div class="command-item">CLICK:MIDDLE - Middle mouse click</div>
                
                <div class="command-category">⏱️ Timing:</div>
                <div class="command-item">DELAY:ms - Wait for milliseconds</div>
                <div class="command-item">WAIT:ms - Same as DELAY</div>
                
                <div class="command-category">📋 Examples:</div>
                <div class="command-item">TYPE:Hello World</div>
                <div class="command-item">ENTER</div>
                <div class="command-item">DELAY:500</div>
                <div class="command-item">COMBO:CTRL+A</div>
                <div class="command-item">COMBO:CTRL+C</div>
                <div class="command-item">WAIT:1000</div>
                <div class="command-item">MOUSE:100,50</div>
                <div class="command-item">CLICK:LEFT</div>
            </div>
        </div>
        
        <!-- Modifier Keys and Layout -->
        <div class="card">
            <h2>🔧 Active Modifiers</h2>
            <div class="modifier-row">
                <button class="modifier-btn" onclick="toggleModifier('ctrl')" id="ctrl-btn">Ctrl</button>
                <button class="modifier-btn" onclick="toggleModifier('shift')" id="shift-btn">Shift</button>
                <button class="modifier-btn" onclick="toggleModifier('alt')" id="alt-btn">Alt</button>
                <button class="modifier-btn" onclick="toggleModifier('gui')" id="gui-btn">Win/Cmd</button>
                <button class="modifier-btn" onclick="toggleModifier('altgr')" id="altgr-btn">AltGr</button>
            </div>
            <div id="combo-display" class="combo-display">No modifiers active</div>
            
            <h3>Keyboard Layout</h3>
            <div class="layout-toggle">
                <button class="layout-btn active-layout" onclick="setLayout('qwertz')" id="qwertz-btn">QWERTZ (DE)</button>
                <button class="layout-btn" onclick="setLayout('qwerty')" id="qwerty-btn">QWERTY (US)</button>
            </div>
        </div>
        
        <!-- Function Keys -->
        <div class="card">
            <h2>F1 - F12</h2>
            <div class="key-row">
                <button class="key fkey" onclick="sendSpecial('f1')">F1</button>
                <button class="key fkey" onclick="sendSpecial('f2')">F2</button>
                <button class="key fkey" onclick="sendSpecial('f3')">F3</button>
                <button class="key fkey" onclick="sendSpecial('f4')">F4</button>
                <button class="key fkey" onclick="sendSpecial('f5')">F5</button>
                <button class="key fkey" onclick="sendSpecial('f6')">F6</button>
                <button class="key fkey" onclick="sendSpecial('f7')">F7</button>
                <button class="key fkey" onclick="sendSpecial('f8')">F8</button>
                <button class="key fkey" onclick="sendSpecial('f9')">F9</button>
                <button class="key fkey" onclick="sendSpecial('f10')">F10</button>
                <button class="key fkey" onclick="sendSpecial('f11')">F11</button>
                <button class="key fkey" onclick="sendSpecial('f12')">F12</button>
            </div>
        </div>
        
        <!-- Navigation & Arrow Keys -->
        <div class="card">
            <div style="display: flex; gap: 20px; justify-content: center; flex-wrap: wrap;">
                <div>
                    <h3>Navigation</h3>
                    <div class="key-row">
                        <button class="key navkey" onclick="sendSpecial('printscreen')">PrtSc</button>
                        <button class="key navkey" onclick="sendSpecial('scrolllock')">ScrLk</button>
                        <button class="key navkey" onclick="sendSpecial('pause')">Pause</button>
                    </div>
                    <div class="key-row" style="margin-top: 5px;">
                        <button class="key navkey" onclick="sendSpecial('insert')">Ins</button>
                        <button class="key navkey" onclick="sendSpecial('home')">Home</button>
                        <button class="key navkey" onclick="sendSpecial('pageup')">PgUp</button>
                    </div>
                    <div class="key-row" style="margin-top: 5px;">
                        <button class="key navkey" onclick="sendSpecial('delete')">Del</button>
                        <button class="key navkey" onclick="sendSpecial('end')">End</button>
                        <button class="key navkey" onclick="sendSpecial('pagedown')">PgDn</button>
                    </div>
                </div>
                
                <div>
                    <h3>Arrow Keys</h3>
                    <div class="nav-arrows">
                        <div></div>
                        <button class="key arrowkey" onclick="sendSpecial('up')">⬆</button>
                        <div></div>
                        <button class="key arrowkey" onclick="sendSpecial('left')">⬅</button>
                        <button class="key arrowkey" onclick="sendSpecial('down')">⬇</button>
                        <button class="key arrowkey" onclick="sendSpecial('right')">➡</button>
                    </div>
                </div>
            </div>
        </div>
        
        <!-- Full Keyboard -->
        <div class="card">
            <h2>⌨️ Main Keyboard</h2>
            <div class="keyboard" id="keyboard"></div>
        </div>
        
        <!-- Mouse Controls -->
        <div class="card">
            <h2>🖱️ Mouse Control</h2>
            <div class="mouse-controls">
                <div></div>
                <button class="preset-btn" onclick="moveMouse(0, -20)">⬆️</button>
                <div></div>
                <button class="preset-btn" onclick="moveMouse(-20, 0)">⬅️</button>
                <button class="preset-btn" onclick="mouseClick('left')">🖱️ Click</button>
                <button class="preset-btn" onclick="moveMouse(20, 0)">➡️</button>
                <div></div>
                <button class="preset-btn" onclick="moveMouse(0, 20)">⬇️</button>
                <div></div>
            </div>
            <div style="text-align: center; margin: 15px 0;">
                <button class="preset-btn" onclick="mouseClick('right')">🖱️ Right Click</button>
                <button class="preset-btn" onclick="mouseClick('middle')">🖱️ Middle Click</button>
            </div>
            <div class="mouse-pad" id="mousePad">
                <div style="text-align: center; padding-top: 50%; color: rgba(255,255,255,0.5);">
                    Touch & drag to move mouse
                </div>
            </div>
        </div>
        
        <!-- Quick Presets -->
        <div class="card">
            <h2>⚡ Quick Actions</h2>
            <div class="preset-grid">
                <button class="preset-btn" onclick="sendCombo(['ctrl','c'])">📋 Copy</button>
                <button class="preset-btn" onclick="sendCombo(['ctrl','v'])">📄 Paste</button>
                <button class="preset-btn" onclick="sendCombo(['ctrl','x'])">✂️ Cut</button>
                <button class="preset-btn" onclick="sendCombo(['ctrl','z'])">↩️ Undo</button>
                <button class="preset-btn" onclick="sendCombo(['ctrl','a'])">📑 Select All</button>
                <button class="preset-btn" onclick="sendCombo(['ctrl','s'])">💾 Save</button>
                <button class="preset-btn" onclick="sendCombo(['alt','tab'])">🔄 Alt+Tab</button>
                <button class="preset-btn" onclick="sendCombo(['ctrl','alt','del'])">🔒 Ctrl+Alt+Del</button>
                <button class="preset-btn" onclick="sendCombo(['gui','r'])">🏃 Win+R</button>
                <button class="preset-btn" onclick="sendCombo(['gui','e'])">📁 Win+E</button>
            </div>
        </div>
        
        <!-- Text Input -->
        <div class="card">
            <h2>💬 Text Input</h2>
            <form id="textForm" onsubmit="sendText(event)">
                <textarea id="textArea" placeholder="Type your text here..."></textarea>
                <button type="submit" class="send-btn">📤 Send Text</button>
            </form>
        </div>
    </div>
    
    <script>
        let activeModifiers = new Set();
        let currentLayout = 'qwertz';
        
        // Keyboard layouts (same as before)
        const layouts = {
            qwertz: {
                normal: [
                    ['`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'ß', '´', {key: 'backspace', label: '⌫', special: true, wide: true}],
                    [{key: 'tab', label: 'Tab', special: true, wide: true}, 'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', 'ü', '+'],
                    [{key: 'capslock', label: 'Caps', special: true, wide: true}, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'ö', 'ä', {key: 'enter', label: 'Enter ↵', special: true, extraWide: true}],
                    [{key: 'shift', label: 'Shift', special: true, extraWide: true}, 'y', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-', {key: 'shift', label: 'Shift', special: true, extraWide: true}],
                    [{key: 'ctrl', label: 'Ctrl', special: true}, {key: 'gui', label: 'Win', special: true}, {key: 'alt', label: 'Alt', special: true}, {key: ' ', label: 'Space', special: false, space: true}, {key: 'altgr', label: 'AltGr', special: true}, {key: 'menu', label: 'Menu', special: true}, {key: 'ctrl', label: 'Ctrl', special: true}]
                ],
                shift: [
                    ['°', '!', '"', '§', '$', '%', '&', '/', '(', ')', '=', '?', '`', {key: 'backspace', label: '⌫', special: true, wide: true}],
                    [{key: 'tab', label: 'Tab', special: true, wide: true}, 'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', 'Ü', '*'],
                    [{key: 'capslock', label: 'Caps', special: true, wide: true}, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'Ö', 'Ä', {key: 'enter', label: 'Enter ↵', special: true, extraWide: true}],
                    [{key: 'shift', label: 'Shift', special: true, extraWide: true}, 'Y', 'X', 'C', 'V', 'B', 'N', 'M', ';', ':', '_', {key: 'shift', label: 'Shift', special: true, extraWide: true}],
                    [{key: 'ctrl', label: 'Ctrl', special: true}, {key: 'gui', label: 'Win', special: true}, {key: 'alt', label: 'Alt', special: true}, {key: ' ', label: 'Space', special: false, space: true}, {key: 'altgr', label: 'AltGr', special: true}, {key: 'menu', label: 'Menu', special: true}, {key: 'ctrl', label: 'Ctrl', special: true}]
                ],
                altgr: [
                    ['`', '1', '²', '³', '4', '5', '6', '{', '[', ']', '}', '\\', '´', {key: 'backspace', label: '⌫', special: true, wide: true}],
                    [{key: 'tab', label: 'Tab', special: true, wide: true}, '@', 'w', '€', 'r', 't', 'z', 'u', 'i', 'o', 'p', 'ü', '~'],
                    [{key: 'capslock', label: 'Caps', special: true, wide: true}, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'ö', 'ä', {key: 'enter', label: 'Enter ↵', special: true, extraWide: true}],
                    [{key: 'shift', label: 'Shift', special: true, extraWide: true}, 'y', 'x', 'c', 'v', 'b', 'n', 'µ', ',', '.', '-', {key: 'shift', label: 'Shift', special: true, extraWide: true}],
                    [{key: 'ctrl', label: 'Ctrl', special: true}, {key: 'gui', label: 'Win', special: true}, {key: 'alt', label: 'Alt', special: true}, {key: ' ', label: 'Space', special: false, space: true}, {key: 'altgr', label: 'AltGr', special: true}, {key: 'menu', label: 'Menu', special: true}, {key: 'ctrl', label: 'Ctrl', special: true}]
                ]
            },
            qwerty: {
                normal: [
                    ['`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', {key: 'backspace', label: '⌫', special: true, wide: true}],
                    [{key: 'tab', label: 'Tab', special: true, wide: true}, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']'],
                    [{key: 'capslock', label: 'Caps', special: true, wide: true}, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', {key: 'enter', label: 'Enter ↵', special: true, extraWide: true}],
                    [{key: 'shift', label: 'Shift', special: true, extraWide: true}, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', {key: 'shift', label: 'Shift', special: true, extraWide: true}],
                    [{key: 'ctrl', label: 'Ctrl', special: true}, {key: 'gui', label: 'Win', special: true}, {key: 'alt', label: 'Alt', special: true}, {key: ' ', label: 'Space', special: false, space: true}, {key: 'altgr', label: 'AltGr', special: true}, {key: 'menu', label: 'Menu', special: true}, {key: 'ctrl', label: 'Ctrl', special: true}]
                ],
                shift: [
                    ['~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', {key: 'backspace', label: '⌫', special: true, wide: true}],
                    [{key: 'tab', label: 'Tab', special: true, wide: true}, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}'],
                    [{key: 'capslock', label: 'Caps', special: true, wide: true}, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', {key: 'enter', label: 'Enter ↵', special: true, extraWide: true}],
                    [{key: 'shift', label: 'Shift', special: true, extraWide: true}, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', {key: 'shift', label: 'Shift', special: true, extraWide: true}],
                    [{key: 'ctrl', label: 'Ctrl', special: true}, {key: 'gui', label: 'Win', special: true}, {key: 'alt', label: 'Alt', special: true}, {key: ' ', label: 'Space', special: false, space: true}, {key: 'altgr', label: 'AltGr', special: true}, {key: 'menu', label: 'Menu', special: true}, {key: 'ctrl', label: 'Ctrl', special: true}]
                ],
                altgr: [
                    ['`', '¡', '™', '£', '¢', '∞', '§', '¶', '•', 'ª', 'º', '–', '≠', {key: 'backspace', label: '⌫', special: true, wide: true}],
                    [{key: 'tab', label: 'Tab', special: true, wide: true}, 'œ', '∑', '´', '®', '†', '¥', '¨', 'ˆ', 'ø', 'π', '«', '»'],
                    [{key: 'capslock', label: 'Caps', special: true, wide: true}, 'å', 'ß', '∂', 'ƒ', '©', '˙', '∆', '˚', '¬', '…', 'æ', {key: 'enter', label: 'Enter ↵', special: true, extraWide: true}],
                    [{key: 'shift', label: 'Shift', special: true, extraWide: true}, 'Ω', '≈', 'ç', '√', '∫', '˜', 'µ', '≤', '≥', '÷', {key: 'shift', label: 'Shift', special: true, extraWide: true}],
                    [{key: 'ctrl', label: 'Ctrl', special: true}, {key: 'gui', label: 'Win', special: true}, {key: 'alt', label: 'Alt', special: true}, {key: ' ', label: 'Space', special: false, space: true}, {key: 'altgr', label: 'AltGr', special: true}, {key: 'menu', label: 'Menu', special: true}, {key: 'ctrl', label: 'Ctrl', special: true}]
                ]
            }
        };
        
        function setLayout(layout) {
            currentLayout = layout;
            document.getElementById('qwertz-btn').classList.remove('active-layout');
            document.getElementById('qwerty-btn').classList.remove('active-layout');
            document.getElementById(layout + '-btn').classList.add('active-layout');
            updateKeyboard();
        }
        
        function toggleModifier(mod) {
            if (activeModifiers.has(mod)) {
                activeModifiers.delete(mod);
                document.getElementById(mod + '-btn').classList.remove('active');
                if (mod === 'altgr') {
                    document.getElementById(mod + '-btn').classList.remove('altgr-active');
                }
            } else {
                activeModifiers.add(mod);
                document.getElementById(mod + '-btn').classList.add('active');
                if (mod === 'altgr') {
                    document.getElementById(mod + '-btn').classList.add('altgr-active');
                }
            }
            updateComboDisplay();
            updateKeyboard();
        }
        
        function updateComboDisplay() {
            const display = document.getElementById('combo-display');
            if (activeModifiers.size === 0) {
                display.textContent = 'No modifiers active';
            } else {
                display.textContent = 'Active: ' + Array.from(activeModifiers).join(' + ');
            }
        }
        
        function updateKeyboard() {
            const keyboardDiv = document.getElementById('keyboard');
            let layoutState = 'normal';
            
            if (activeModifiers.has('altgr')) {
                layoutState = 'altgr';
            } else if (activeModifiers.has('shift')) {
                layoutState = 'shift';
            }
            
            const currentKeys = layouts[currentLayout][layoutState];
            
            let keyboardHTML = '';
            currentKeys.forEach(row => {
                keyboardHTML += '<div class="key-row">';
                row.forEach(keyData => {
                    if (typeof keyData === 'object') {
                        let classes = 'key';
                        if (keyData.special) classes += ' special';
                        if (keyData.wide) classes += ' wide';
                        if (keyData.extraWide) classes += ' extra-wide';
                        if (keyData.space) classes += ' extra-wide';
                        
                        let onClick = '';
                        if (keyData.special) {
                            onClick = `onclick="sendSpecial('${keyData.key}')"`;
                        } else {
                            onClick = `onclick="sendKey('${keyData.key}')"`;
                        }
                        
                        let style = '';
                        if (keyData.space) {
                            style = 'style="min-width: 250px;"';
                        }
                        
                        keyboardHTML += `<button class="${classes}" ${onClick} ${style}>${keyData.label}</button>`;
                    } else {
                        keyboardHTML += `<button class="key" onclick="sendKey('${keyData}')">${keyData}</button>`;
                    }
                });
                keyboardHTML += '</div>';
            });
            
            keyboardDiv.innerHTML = keyboardHTML;
        }
        
        function toggleCommands() {
            const cmdList = document.getElementById('commandList');
            cmdList.classList.toggle('visible');
        }
        
        async function executeScript() {
            const script = document.getElementById('scriptArea').value;
            if (!script.trim()) {
                showStatus('❌ Script is empty!');
                return;
            }
            
            const lines = script.split('\n');
            showStatus('▶️ Executing script...');
            
            for (const line of lines) {
                const trimmedLine = line.trim();
                if (!trimmedLine || trimmedLine.startsWith('#')) continue;
                
                try {
                    const response = await fetch('/script', {
                        method: 'POST',
                        headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                        body: 'command=' + encodeURIComponent(trimmedLine)
                    });
                    
                    if (!response.ok) {
                        throw new Error('Server error');
                    }
                    
                    // Small delay between commands
                    await new Promise(resolve => setTimeout(resolve, 50));
                } catch (error) {
                    showStatus('❌ Error executing: ' + trimmedLine);
                    return;
                }
            }
            
            showStatus('✅ Script executed successfully!');
        }
        
        async function sendKey(key) {
            const modifiers = Array.from(activeModifiers);
            try {
                const response = await fetch('/key', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'key=' + encodeURIComponent(key) + '&modifiers=' + encodeURIComponent(modifiers.join(','))
                });
                if (response.ok) {
                    showStatus('✅ Key sent: ' + (modifiers.length ? modifiers.join('+') + '+' : '') + key);
                }
            } catch (error) {
                showStatus('❌ Error sending key');
            }
        }
        
        async function sendSpecial(key) {
            const modifiers = Array.from(activeModifiers);
            try {
                const response = await fetch('/special', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'key=' + key + '&modifiers=' + encodeURIComponent(modifiers.join(','))
                });
                if (response.ok) {
                    showStatus('✅ Special key sent: ' + key);
                }
            } catch (error) {
                showStatus('❌ Error sending key');
            }
        }
        
        async function sendCombo(keys) {
            try {
                const response = await fetch('/combo', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'keys=' + encodeURIComponent(keys.join(','))
                });
                if (response.ok) {
                    showStatus('✅ Combo sent: ' + keys.join(' + '));
                }
            } catch (error) {
                showStatus('❌ Error sending combo');
            }
        }
        
        async function moveMouse(x, y) {
            try {
                const response = await fetch('/mouse', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'x=' + x + '&y=' + y + '&type=move'
                });
                if (response.ok) {
                    showStatus('✅ Mouse moved: (' + x + ', ' + y + ')');
                }
            } catch (error) {
                showStatus('❌ Error moving mouse');
            }
        }
        
        async function mouseClick(button) {
            try {
                const response = await fetch('/mouse', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'type=click&button=' + button
                });
                if (response.ok) {
                    showStatus('✅ Mouse ' + button + ' click');
                }
            } catch (error) {
                showStatus('❌ Error with mouse click');
            }
        }
        
        async function sendText(event) {
            event.preventDefault();
            const text = document.getElementById('textArea').value;
            if (!text) return;
            
            try {
                const response = await fetch('/type', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'text=' + encodeURIComponent(text)
                });
                if (response.ok) {
                    showStatus('✅ Text sent!');
                    document.getElementById('textArea').value = '';
                }
            } catch (error) {
                showStatus('❌ Error sending text');
            }
        }
        
        function showStatus(message) {
            const status = document.getElementById('status');
            status.textContent = message;
            status.style.display = 'block';
            setTimeout(() => {
                status.style.display = 'none';
            }, 2000);
        }
        
        // Mouse pad
        const mousePad = document.getElementById('mousePad');
        let lastX, lastY;
        
        mousePad.addEventListener('mousedown', (e) => {
            lastX = e.clientX;
            lastY = e.clientY;
            mousePad.style.cursor = 'grabbing';
        });
        
        mousePad.addEventListener('mousemove', (e) => {
            if (e.buttons === 1) {
                const dx = e.clientX - lastX;
                const dy = e.clientY - lastY;
                moveMouse(dx, dy);
                lastX = e.clientX;
                lastY = e.clientY;
            }
        });
        
        mousePad.addEventListener('mouseup', () => {
            mousePad.style.cursor = 'crosshair';
        });
        
        mousePad.addEventListener('mouseleave', () => {
            mousePad.style.cursor = 'crosshair';
        });
        
        mousePad.addEventListener('touchstart', (e) => {
            e.preventDefault();
            lastX = e.touches[0].clientX;
            lastY = e.touches[0].clientY;
        });
        
        mousePad.addEventListener('touchmove', (e) => {
            e.preventDefault();
            const dx = e.touches[0].clientX - lastX;
            const dy = e.touches[0].clientY - lastY;
            moveMouse(dx, dy);
            lastX = e.touches[0].clientX;
            lastY = e.touches[0].clientY;
        });
        
        document.addEventListener('keydown', (e) => {
            if (e.key === 'Control' && !e.repeat) toggleModifier('ctrl');
            if (e.key === 'Shift' && !e.repeat) toggleModifier('shift');
            if (e.key === 'Alt' && !e.repeat) toggleModifier('alt');
            if (e.key === 'Meta' && !e.repeat) toggleModifier('gui');
        });
        
        document.addEventListener('keyup', (e) => {
            if (e.key === 'Control' && activeModifiers.has('ctrl')) toggleModifier('ctrl');
            if (e.key === 'Shift' && activeModifiers.has('shift')) toggleModifier('shift');
            if (e.key === 'Alt' && activeModifiers.has('alt')) toggleModifier('alt');
            if (e.key === 'Meta' && activeModifiers.has('gui')) toggleModifier('gui');
        });
        
        // Initialize keyboard
        updateKeyboard();
    </script>
</body>
</html>
)rawliteral";
}

// ---------- Helper function for modifier keys ----------
void pressModifierKey(String key) {
    if (key == "ctrl") Keyboard.press(KEY_LEFT_CTRL);
    else if (key == "shift") Keyboard.press(KEY_LEFT_SHIFT);
    else if (key == "alt") Keyboard.press(KEY_LEFT_ALT);
    else if (key == "gui") Keyboard.press(KEY_LEFT_GUI);
    else if (key == "altgr") Keyboard.press(KEY_RIGHT_ALT);
    else if (key == "tab") Keyboard.press(KEY_TAB);
    else if (key == "enter") Keyboard.press(KEY_RETURN);
    else if (key == "backspace") Keyboard.press(KEY_BACKSPACE);
    else if (key == "delete") Keyboard.press(KEY_DELETE);
    else if (key == "escape") Keyboard.press(KEY_ESC);
    else if (key == "del") Keyboard.press(KEY_DELETE);
    else if (key.length() == 1) Keyboard.press(key[0]);
}

// ---------- Execute Script Command ----------
void executeScript(String command) {
    command.trim();
    if (command.length() == 0 || command.startsWith("#")) return;
    
    int colonPos = command.indexOf(':');
    String cmd = command;
    String arg = "";
    
    if (colonPos > 0) {
        cmd = command.substring(0, colonPos);
        arg = command.substring(colonPos + 1);
    }
    
    cmd.toUpperCase();
    arg.trim();
    
    Serial.println("Script command: " + cmd + " Arg: " + arg);
    
    // Text commands
    if (cmd == "TYPE" || cmd == "STRING") {
        if (arg.length() > 0) {
            Keyboard.print(arg);
        }
    }
    // Special keys
    else if (cmd == "ENTER") Keyboard.write(KEY_RETURN);
    else if (cmd == "TAB") Keyboard.write(KEY_TAB);
    else if (cmd == "SPACE") Keyboard.write(' ');
    else if (cmd == "BACKSPACE") Keyboard.write(KEY_BACKSPACE);
    else if (cmd == "DELETE" || cmd == "DEL") Keyboard.write(KEY_DELETE);
    else if (cmd == "ESCAPE" || cmd == "ESC") Keyboard.write(KEY_ESC);
    else if (cmd == "CAPSLOCK") Keyboard.write(KEY_CAPS_LOCK);
    else if (cmd == "MENU") {
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.write(KEY_F10);
        Keyboard.release(KEY_LEFT_SHIFT);
    }
    
    // Function keys
    else if (cmd == "F1") Keyboard.write(KEY_F1);
    else if (cmd == "F2") Keyboard.write(KEY_F2);
    else if (cmd == "F3") Keyboard.write(KEY_F3);
    else if (cmd == "F4") Keyboard.write(KEY_F4);
    else if (cmd == "F5") Keyboard.write(KEY_F5);
    else if (cmd == "F6") Keyboard.write(KEY_F6);
    else if (cmd == "F7") Keyboard.write(KEY_F7);
    else if (cmd == "F8") Keyboard.write(KEY_F8);
    else if (cmd == "F9") Keyboard.write(KEY_F9);
    else if (cmd == "F10") Keyboard.write(KEY_F10);
    else if (cmd == "F11") Keyboard.write(KEY_F11);
    else if (cmd == "F12") Keyboard.write(KEY_F12);
    
    // Navigation
    else if (cmd == "HOME") Keyboard.write(KEY_HOME);
    else if (cmd == "END") Keyboard.write(KEY_END);
    else if (cmd == "PAGEUP" || cmd == "PGUP") Keyboard.write(KEY_PAGE_UP);
    else if (cmd == "PAGEDOWN" || cmd == "PGDN") Keyboard.write(KEY_PAGE_DOWN);
    else if (cmd == "INSERT" || cmd == "INS") Keyboard.write(KEY_INSERT);
    else if (cmd == "PRINTSCREEN" || cmd == "PRTSC") Keyboard.write(KEY_F13);
    else if (cmd == "SCROLLLOCK") Keyboard.write(KEY_F14);
    else if (cmd == "PAUSE") Keyboard.write(KEY_F15);
    
    // Arrow keys
    else if (cmd == "UP") Keyboard.write(KEY_UP_ARROW);
    else if (cmd == "DOWN") Keyboard.write(KEY_DOWN_ARROW);
    else if (cmd == "LEFT") Keyboard.write(KEY_LEFT_ARROW);
    else if (cmd == "RIGHT") Keyboard.write(KEY_RIGHT_ARROW);
    
    // Modifier keys (press and release)
    else if (cmd == "CTRL") {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.release(KEY_LEFT_CTRL);
    }
    else if (cmd == "SHIFT") {
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.release(KEY_LEFT_SHIFT);
    }
    else if (cmd == "ALT") {
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.release(KEY_LEFT_ALT);
    }
    else if (cmd == "GUI" || cmd == "WIN" || cmd == "CMD") {
        Keyboard.press(KEY_LEFT_GUI);
        Keyboard.release(KEY_LEFT_GUI);
    }
    else if (cmd == "ALTGR") {
        Keyboard.press(KEY_RIGHT_ALT);
        Keyboard.release(KEY_RIGHT_ALT);
    }
    
    // Key combinations
    else if (cmd == "COMBO") {
        if (arg.length() > 0) {
            // Parse combo keys separated by +
            String keys[10];
            int keyCount = 0;
            int start = 0;
            int plusPos = arg.indexOf('+');
            
            while (plusPos > 0 && keyCount < 10) {
                keys[keyCount] = arg.substring(start, plusPos);
                keys[keyCount].trim();
                keys[keyCount].toUpperCase();
                keyCount++;
                start = plusPos + 1;
                plusPos = arg.indexOf('+', start);
            }
            // Last key
            keys[keyCount] = arg.substring(start);
            keys[keyCount].trim();
            keys[keyCount].toUpperCase();
            keyCount++;
            
            // Press all keys
            for (int i = 0; i < keyCount; i++) {
                pressModifierKey(keys[i]);
            }
            delay(50);
            Keyboard.releaseAll();
        }
    }
    
    // Mouse commands
    else if (cmd == "MOUSE") {
        if (arg.length() > 0) {
            int commaPos = arg.indexOf(',');
            if (commaPos > 0) {
                int x = arg.substring(0, commaPos).toInt();
                int y = arg.substring(commaPos + 1).toInt();
                Mouse.move(x, y, 0);
            }
        }
    }
    else if (cmd == "CLICK") {
        if (arg == "LEFT") Mouse.click(MOUSE_LEFT);
        else if (arg == "RIGHT") Mouse.click(MOUSE_RIGHT);
        else if (arg == "MIDDLE") Mouse.click(MOUSE_MIDDLE);
    }
    
    // Timing
    else if (cmd == "DELAY" || cmd == "WAIT") {
        int ms = arg.toInt();
        if (ms > 0 && ms <= 10000) { // Max 10 seconds
            delay(ms);
        }
    }
    
    // Single character
    else if (cmd.length() == 1) {
        Keyboard.write(cmd[0]);
    }
}

// ---------- Handle keyboard key ----------
void handleKey() {
    if (server.hasArg("key")) {
        String key = server.arg("key");
        String modifiers = server.arg("modifiers");
        
        if (modifiers.indexOf("ctrl") >= 0) Keyboard.press(KEY_LEFT_CTRL);
        if (modifiers.indexOf("shift") >= 0) Keyboard.press(KEY_LEFT_SHIFT);
        if (modifiers.indexOf("alt") >= 0) Keyboard.press(KEY_LEFT_ALT);
        if (modifiers.indexOf("gui") >= 0) Keyboard.press(KEY_LEFT_GUI);
        if (modifiers.indexOf("altgr") >= 0) Keyboard.press(KEY_RIGHT_ALT);
        
        if (key.length() == 1) {
            Keyboard.write(key[0]);
        }
        
        Keyboard.releaseAll();
    }
    server.send(200, "text/plain", "OK");
}

// ---------- Handle special keys ----------
void handleSpecial() {
    if (server.hasArg("key")) {
        String key = server.arg("key");
        String modifiers = server.arg("modifiers");
        
        if (modifiers.indexOf("ctrl") >= 0) Keyboard.press(KEY_LEFT_CTRL);
        if (modifiers.indexOf("shift") >= 0) Keyboard.press(KEY_LEFT_SHIFT);
        if (modifiers.indexOf("alt") >= 0) Keyboard.press(KEY_LEFT_ALT);
        if (modifiers.indexOf("gui") >= 0) Keyboard.press(KEY_LEFT_GUI);
        if (modifiers.indexOf("altgr") >= 0) Keyboard.press(KEY_RIGHT_ALT);
        
        if (key == "f1") Keyboard.write(KEY_F1);
        else if (key == "f2") Keyboard.write(KEY_F2);
        else if (key == "f3") Keyboard.write(KEY_F3);
        else if (key == "f4") Keyboard.write(KEY_F4);
        else if (key == "f5") Keyboard.write(KEY_F5);
        else if (key == "f6") Keyboard.write(KEY_F6);
        else if (key == "f7") Keyboard.write(KEY_F7);
        else if (key == "f8") Keyboard.write(KEY_F8);
        else if (key == "f9") Keyboard.write(KEY_F9);
        else if (key == "f10") Keyboard.write(KEY_F10);
        else if (key == "f11") Keyboard.write(KEY_F11);
        else if (key == "f12") Keyboard.write(KEY_F12);
        else if (key == "printscreen") Keyboard.write(KEY_F13);
        else if (key == "scrolllock") Keyboard.write(KEY_F14);
        else if (key == "pause") Keyboard.write(KEY_F15);
        else if (key == "insert") Keyboard.write(KEY_INSERT);
        else if (key == "home") Keyboard.write(KEY_HOME);
        else if (key == "pageup") Keyboard.write(KEY_PAGE_UP);
        else if (key == "delete") Keyboard.write(KEY_DELETE);
        else if (key == "end") Keyboard.write(KEY_END);
        else if (key == "pagedown") Keyboard.write(KEY_PAGE_DOWN);
        else if (key == "up") Keyboard.write(KEY_UP_ARROW);
        else if (key == "down") Keyboard.write(KEY_DOWN_ARROW);
        else if (key == "left") Keyboard.write(KEY_LEFT_ARROW);
        else if (key == "right") Keyboard.write(KEY_RIGHT_ARROW);
        else if (key == "enter") Keyboard.write(KEY_RETURN);
        else if (key == "tab") Keyboard.write(KEY_TAB);
        else if (key == "backspace") Keyboard.write(KEY_BACKSPACE);
        else if (key == "escape") Keyboard.write(KEY_ESC);
        else if (key == "capslock") Keyboard.write(KEY_CAPS_LOCK);
        else if (key == "shift") Keyboard.write(KEY_LEFT_SHIFT);
        else if (key == "ctrl") Keyboard.write(KEY_LEFT_CTRL);
        else if (key == "alt") Keyboard.write(KEY_LEFT_ALT);
        else if (key == "gui") Keyboard.write(KEY_LEFT_GUI);
        else if (key == "altgr") Keyboard.write(KEY_RIGHT_ALT);
        else if (key == "menu") {
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.write(KEY_F10);
            Keyboard.release(KEY_LEFT_SHIFT);
        }
        
        Keyboard.releaseAll();
    }
    server.send(200, "text/plain", "OK");
}

// ---------- Handle key combos ----------
void handleCombo() {
    if (server.hasArg("keys")) {
        String keysStr = server.arg("keys");
        int start = 0;
        int end = keysStr.indexOf(',');
        
        while (end >= 0) {
            String key = keysStr.substring(start, end);
            pressModifierKey(key);
            start = end + 1;
            end = keysStr.indexOf(',', start);
        }
        String lastKey = keysStr.substring(start);
        pressModifierKey(lastKey);
        
        delay(50);
        Keyboard.releaseAll();
    }
    server.send(200, "text/plain", "OK");
}

// ---------- Handle script commands ----------
void handleScript() {
    if (server.hasArg("command")) {
        String command = server.arg("command");
        executeScript(command);
    }
    server.send(200, "text/plain", "OK");
}

// ---------- Handle mouse movements ----------
void handleMouse() {
    if (server.hasArg("type")) {
        String type = server.arg("type");
        
        if (type == "move") {
            int x = server.arg("x").toInt();
            int y = server.arg("y").toInt();
            Mouse.move(x, y, 0);
        }
        else if (type == "click") {
            String button = server.arg("button");
            if (button == "left") Mouse.click(MOUSE_LEFT);
            else if (button == "right") Mouse.click(MOUSE_RIGHT);
            else if (button == "middle") Mouse.click(MOUSE_MIDDLE);
        }
    }
    server.send(200, "text/plain", "OK");
}

// ---------- Handle typing ----------
void handleType() {
    if (server.hasArg("text")) {
        String text = server.arg("text");
        Keyboard.print(text);
    }
    server.send(200, "text/plain", "OK");
}

// ---------- Setup ----------
void setup() {
    Serial.begin(115200);
    
    led.begin();
    setLED(255, 0, 0);
    
    USB.begin();
    Keyboard.begin();
    Mouse.begin();
    
    delay(1000);
    
    WiFi.softAP(ssid, password);
    
    Serial.println();
    Serial.println("WiFi started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
    setLED(0, 0, 255);
    
    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", html());
    });
    
    server.on("/key", HTTP_POST, handleKey);
    server.on("/special", HTTP_POST, handleSpecial);
    server.on("/combo", HTTP_POST, handleCombo);
    server.on("/script", HTTP_POST, handleScript);
    server.on("/mouse", HTTP_POST, handleMouse);
    server.on("/type", HTTP_POST, handleType);
    
    server.begin();
    Serial.println("Web server ready!");
    Serial.println("Open http://" + WiFi.softAPIP().toString() + " in your browser");
}

void loop() {
    server.handleClient();
}
