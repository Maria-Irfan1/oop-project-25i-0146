# Image Filter Studio 🛠️📸

A high-performance, console-based image processing engine built from scratch in Object-Oriented C++. The application decodes real JPG and PNG image structures into memory, exposes them via low-level heap pointers, applies sequential filter transformations using polymorphism, and writes the output as a fully readable file back to disk.

This project was built to explore core software engineering principles, avoiding heavy third-party frameworks or high-level automatic containers in favor of raw pointer manipulation and deterministic memory tracking.

---

## 🚀 Architectural Breakdown

### 1. Low-Level Memory Handling & Pixel Core
* **Explicit Resource Management:** To achieve true deterministic optimization, the core image structure avoids automated resizing vectors. It is mapped to a dynamic 2D array of `Pixel` objects (`Pixel** grid`)[cite: 3].
* **Deep Copying Enforcement:** Features a custom deep-copy constructor and an overloaded assignment operator (`=`) to securely track heap references and prevent dangling pointer crashes or double-free errors[cite: 3].
* **Value Clamping:** Features strict value validation handling that prevents overflow/underflow anomalies across color channels by clamping them precisely between `0-255`[cite: 3].

### 2. Polymorphic Filter Pipeline Engine
The filter suite relies entirely on runtime polymorphism, inheriting from an abstract base `Filter` class[cite: 3]. Users can stack and chain multiple distinct operations into a single continuous `FilterSession` pipeline execution stream[cite: 3]:
* **Spatial Filters:** Features a `BoxBlur` filter that softens image grids using spatial convolution over a localized `3x3` matrix[cite: 3].
* **Pixel Intensity Modifiers:** Includes dynamic scaling engines for mathematical contrast stretching, negative inversion, grayscale conversion, and brightness calibration[cite: 3].
* **Geometric Transpositions:** Implements custom pixel coordinate mapping algorithms enabling lossless horizontal and vertical grid reflections[cite: 3].
* **Channel Isolators:** Discretized RGB color channel extractions (`RedChannelOnly`, `GreenChannelOnly`, `BlueChannelOnly`) that isolate specific color bands[cite: 3].

### 3. State Management & ASCII Engine
* **Real-Time Visual Feedback:** Features an overloaded stream insertion operator (`<<`) mapping live pixel color channels down to a relative brightness scale, generating interactive ASCII art thumbnail updates inside the console window as each step completes execution[cite: 3].
* **File-Based Persistence Database:** Implements pipe-delimited text files acting as a database tracker logging user credential schemas (`customers.txt`), timestamped system activity sessions (`sessions.txt`), and toggled configuration parameters (`catalog.txt`)[cite: 3].
* **User Authentication Framework:** Implements separate dashboards for `Admin` and `Customer` roles, complete with strict password validation constraints (requiring uppercase and digits) and a 3-attempt brute-force mitigation limit for customer logins[cite: 3].

---

## 🛠️ Tech Stack & Dependencies

* **Language:** Pure C++ (Standard C++11 or higher)[cite: 3]
* **Image Decoders/Encoders:** Integrated with `stb_image.h` and `stb_image_write.h` to read/write binary buffers into custom memory matrices[cite: 3].
* **Database System:** Custom file parsers built on standard file streams (`std::ifstream`, `std::ofstream`)[cite: 3].
* **Build Configuration:** Compiles across UNIX and Windows environments via `g++`[cite: 3].

---

## ⚙️ Compilation & Setup

To compile and launch the executable studio from your terminal environment, execute the following commands:

```bash
# Navigate to the project directory
cd ooplab-project

# Compile via g++
g++ -o FilterStudio main.cpp

# Run the program
./FilterStudio
