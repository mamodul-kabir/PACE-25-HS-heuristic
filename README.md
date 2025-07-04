# PACE 2025 Heuristic Hitting Set Solver

This repository contains the code for a **Hitting Set problem solver** developed for the **heuristic track of the PACE 2025 Challenge**.

---


## Requirements

- C++ Standard: C++23 (recommended)
- CMake minimum version: 3.10
- Git LFS: Required to download large model files (e.g., `.onnx`)

---

## Installation

1. **Clone the repository and pull LFS files:**

   ```bash
   git clone https://github.com/mamodul-kabir/PACE-25-HS-heuristic.git
   cd PACE-25-HS-heuristic

   # Initialize Git LFS and pull large files
   git lfs install
   git lfs pull
   ```

2. **Build NetworKit:**
	```bash
	cd networkit
	mkdir build && cd build
	cmake ..
	make -j$(nproc)
	```
3. **Build the main project:**
	```bash
	cd ../..
	mkdir build && cd build
	cmake ..
	make -j$(nproc)
	```
---

## Running the Program

Run the solver on a hitting set instance file (`.hgr` or `.txt`) using:
```bash
./pace < input.hgr
```

To save in a file, write: 
```bash
./pace < input.hgr > output.txt
```
---

## Machine Learning Model (Optional)
**This section is completely optional and is not required to use the solver.**

- A pre-trained machine learning model `(rf_model.onnx)` is provided in the repository.
- If you want to train the model manually, follow the steps below.
- The files used to train the current model can be found in `training/` directory. 


### Training and Using the Machine Learning Model 

1. Create a Python virtual environment and activate it:
	```bash
	python3 -m venv venv
	source venv/bin/activate
	```
3. Install dependencies:
	```bash
	pip install -r classifier/requirements.txt
	```
4. Run the training script:
	```bash
	python3 classifier/random_forest_classifier.py
	python3 classifier/convert_rf_to_onnx.py
	```

### Generating CSV Files for Training

You can generate CSV training files using the function in `pace.h`.

#### Example:
Modify `src/main.cpp`:
```cpp
#include "pace.h"

int main() {
    generate_csv(3600.0);  // time limit in seconds, e.g. 1 hour
    return 0;
}
```
Compile and run:
```bash
cd PACE-25-HS-heuristic/build
make -j$(nproc)
./pace < training_instance.hgr > output_training.csv
```
---

## Third-party Notice

This project uses third-party components:

- **NetworKit** (https://github.com/networkit/networkit) – MIT License
- **ONNX Runtime** (https://github.com/microsoft/onnxruntime) – MIT License
- **NuSC-Algorithm / wscp.h** (https://github.com/chuanluocs/NuSC-Algorithm) – GPL-3.0 License

The file `src/wscp.h` is adapted from the NuSC-Algorithm project and is licensed under the GNU General Public License v3.0.