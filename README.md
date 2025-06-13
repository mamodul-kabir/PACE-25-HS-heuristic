# PACE 2025 Heuristic Hitting Set Solver

This repository contains the code for a **Hitting Set problem solver** developed for the **heuristic track of the PACE 2025 Challenge**.

---

## Important Notes

- The **HiGHS** library is **not** used to solve the Hitting Set instances directly.
- HiGHS is used only to generate CSV files for training the machine learning model.

---

## Requirements

- C++ Standard: C++23 (recommended)
- CMake minimum version: 3.10

---

## Installation

1. **Clone the repository:**

   ```bash
   git clone https://github.com/mamodul-kabir/PACE-25-HS-heuristic.git
   cd PACE-25-HS-heuristic
   ```

2. **Build HiGHS:**
	```bash
	cd HiGHS
	mkdir build && cd build
	cmake ..
	make -j$(nproc)
	```
3. **Build NetworKit:**
	```bash
	cd ../../networkit
	mkdir build && cd build
	cmake ..
	make -j$(nproc)
	```
4. **Build the main project:**
	```bash
	cd ../..
	mkdir build && cd build
	cmake ..
	make -j$(nproc)
	```
---

## Running the Program

Run the solver on a hitting set instance file (.hgr or .txt) as:
```bash
./pace < input.hgr
```

To save in a file, write: 
```bash
./pace < input.hgr > output.txt
```
---

## Machine Learning Model

- A pre-trained machine learning model `(random_forest_model.onnx)` is provided in the repository.
- If you want to train the model manually, follow the steps below.
- The files used to train the current model can be found in `training\` directory. 

---

### Training and Using the Machine Learning Model (Optional)

1. **Create a Python virtual environment and activate it:**
	```bash
	python3 -m venv venv
	source venv/bin/activate
	```
3. **Install dependencies:**
	```bash
	pip install -r classifier/requirements.txt
	```
4. **Run the training script:**
	```bash
	python classifier/random_forest_classifier.py
	```
---

### Generating CSV Files for Training (Optional)

You can generate CSV training files using the function in pace.h.

#### Example:
Create a file train.cpp:
```cpp
#include "pace.h"

int main() {
    loadInp();
    generate_csv(3600.0);  // time limit in seconds, e.g. 1 hour
    return 0;
}
```
Compile and run:
```bash
g++ train.cpp -o train
./train < training_instance.hgr > training.csv
```

## Third-party Notice

This project uses third-party components:

- **HiGHS** (https://github.com/ERGO-Code/HiGHS) – MIT License
- **NetworKit** (https://github.com/networkit/networkit) – MIT License
- **ONNX Runtime** (https://github.com/microsoft/onnxruntime) – MIT License
- **NuSC-Algorithm / wscp.h** (https://github.com/chuanluocs/NuSC-Algorithm) – GPL-3.0 License

The file `src/wscp.h` is adapted from the NuSC-Algorithm project and is licensed under the GNU General Public License v3.0.