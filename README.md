# WiFi-Based Interactive Museum Guide (M5Stack CoreS3)

This project transforms an **M5Stack CoreS3 (Tab5)** into an intelligent interactive guide for the Cité des Sciences. It uses WiFi fingerprinting and a Neural Network to detect a visitor's location and display relevant artwork information (images) automatically.



## 🚀 Project Overview
The system operates in three distinct phases:
1.  **Data Collection**: Scanning and logging WiFi RSSI (signal strength) for 10 specific zones using the Tab5.
2.  **Machine Learning**: Processing the collected data via Google Colab to train an Artificial Neural Network (ANN).
3.  **Real-time Restitution**: The tablet receives a predicted zone via MQTT and displays the corresponding artwork image from the SD card.

---

## 📂 Repository Structure (Project Plan)

Based on the technical roadmap, the repository is organized as follows:

* **`README.md`**: Project overview and documentation.
* **`Troubleshooting/`**: FAQ and solutions for common hardware/software issues.
* **`Google_Colab/`**: The `.ipynb` notebook for data cleaning, training, and generating the `ANN` weights.
* **`Arduino_Codes/`**:
    * `Data_Collector.ino`: Code for WiFi signal gathering.
    * `Visitor_Tab_Restitution.ino`: Final code for the guide tablet (Restitution).
* **`Node-RED/`**: JSON export of the Node-RED flow used for the MQTT broker and CSV logging.
* **`Data/`**:
    * `dataset.csv`: Collected WiFi MAC addresses and RSSI levels.
    * `model_weights.json`: The Neural Network weights (ANN) for on-device or cloud inference.

---

## 🛠 Hardware Requirements
* **M5Stack CoreS3 (Tab5)** (ESP32-S3)
* **Micro SD Card**: Must be formatted in **FAT32**.
* **Broker**: HiveMQ Cloud (MQTT)
* **Node RED**

---

## 🔧 Troubleshooting (FAQ)

### **1. Why is the screen RED at startup?**
**Answer:** This indicates the SD Card was not detected.
**Solution:** Ensure the card is formatted to **FAT32** and inserted firmly into the slot.

### **2. Why does the code fail to compile with `drawJpgFile`?**
**Answer:** There is a known "abstract type" error in the M5GFX library when using `SD_MMC`.
**Solution:** Use the "RAM Buffer" method: open the file manually, read it into memory (`uint8_t*`), and then call `M5.Lcd.drawJpg(buffer, size, 0, 0)`.

### **3. The image is not displaying (Black screen).**
**Answer:** The image format might be incompatible.
**Solution:** Images must be **JPG or PNG** (Baseline/Standard, not Progressive) and should be resized to **480x272 pixels** to fit the RAM.

---

## 📡 Connectivity
* **MQTT Topic**: `LaVilette/sciencz/pos` and   `LaVilette/science/wifi/data`

---

## 👨‍💻 Author
Developed as part of the Cité des Sciences Interactive Guide projecT.
