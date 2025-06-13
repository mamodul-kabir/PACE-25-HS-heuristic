# PACE 2025 Heuristic Hitting Set Solver

This repository contains the code for a **Hitting Set problem solver** developed for the **heuristic track of the PACE 2025 Challenge**.

---

## Important Notes

- The **HiGHS** library is **not** used to solve the Hitting Set instances directly. It is only used to generate CSV files for training the machine learning model.
- The solver incorporates code from the **NuSC** algorithm (see the [Third-party Notice](#third-party-notice)).

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

Run the solver on a hitting set instance file (`.hgr` or `.txt`) using:
```bash
./pace < input.hgr
```

To save in a file, write: 
```bash
./pace < input.hgr > output.txt
```
---

## Solver Description

### Overview

- The solver begins by loading the instance and constructing a hypergraph from the input.
- For each element in the hypergraph, a set of features is computed based on the graph structure, including degree-based and neighborhood-based statistics.
- A pre-trained Random Forest classifier (exported to ONNX format) is then used to evaluate each element:
  - If the model predicts the element is not part of the hitting set with high confidence (≥ 0.6), it is excluded from further consideration.
  - If the model predicts the element is part of the hitting set with high confidence (≥ 0.65), it is immediately added to the solution.
- After pruning, the solver remaps the reduced problem and solves the remaining instance using the NuSC algorithm.

### Runtime Strategy

- The total solver time is capped at 250 seconds to remain within the five-minute runtime constraint.
- The pruning and preprocessing phase is timed precisely, and the remaining time is passed as a budget to NuSC.

---

## Machine Learning Model (Optional)
**This section is completely optional and is not required to use the solver.**

- A pre-trained machine learning model `(random_forest_model.onnx)` is provided in the repository.
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
	python classifier/random_forest_classifier.py
	```

### Generating CSV Files for Training

You can generate CSV training files using the function in `pace.h`.

#### Example:
Modify `src/main.cpp`:
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
cd PACE-25-HS-heuristic/build
make -j$(nproc)
./pace < training_instance.hgr > output_training.csv
```
---

## Third-party Notice

This project uses third-party components:

- **HiGHS** (https://github.com/ERGO-Code/HiGHS) – MIT License
- **NetworKit** (https://github.com/networkit/networkit) – MIT License
- **ONNX Runtime** (https://github.com/microsoft/onnxruntime) – MIT License
- **NuSC-Algorithm / wscp.h** (https://github.com/chuanluocs/NuSC-Algorithm) – GPL-3.0 License

The file `src/wscp.h` is adapted from the NuSC-Algorithm project and is licensed under the GNU General Public License v3.0.