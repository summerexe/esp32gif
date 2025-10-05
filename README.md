# 🎞️ ESP32 OLED GIF Display

Display your favorite GIFs on a 128×64 OLED (SSD1306) using an ESP32!

Created by [@summer_exe](https://www.tiktok.com/@summer_exe) on TikTok  
and [@summerr.exe](https://www.instagram.com/summerr.exe) on Instagram 🌞

---

## 🧩 Components Needed
- ESP32 board (any DevKit version)
- 128×64 OLED display (SSD1306, I²C type)
- Jumper wires

---

## ⚡ Wiring
| OLED Pin | ESP32 Pin | Note |
|-----------|------------|------|
| VCC | 3.3V | Power |
| GND | GND | Ground |
| SDA | GPIO 21 | I²C data |
| SCL | GPIO 22 | I²C clock |

---

## 🧰 Setup (PlatformIO)
1. Install **VS Code** and the **PlatformIO extension**.  
2. Clone or download this repository.  
3. Open the project folder in VS Code → PlatformIO.  
4. Connect your ESP32 via USB.  
5. Select the correct serial port and click **Upload (▶️)**.

---

## 🎨 Steps to Create Your Own GIF Animation

### **Step 1 – Pick your GIF**
Go to [giphy.com](https://giphy.com) and download the GIF you like.

### **Step 2 – Split the GIF into frames**
1. Visit [https://ezgif.com/split](https://ezgif.com/split)  
2. Upload your GIF  
3. Click **“Split to frames”** and download all frames  
4. Resize each frame to **128×64 pixels**

### **Step 3 – Convert frames to C arrays**
1. Go to [Image2CPP](https://javl.github.io/image2cpp/)  
2. Upload each frame (one by one)  
3. Choose:
   - Output: **Arduino code**
   - Mode: **Monochrome**
   - Size: **128×64**
4. Copy the generated C array code.

### **Step 4 – Paste your frames**
1. Open `src/main.cpp`  
2. Paste each frame under the “BITMAP FRAMES” section:
   ```cpp
   const unsigned char frame1[] PROGMEM = { ... };
   const unsigned char frame2[] PROGMEM = { ... };
