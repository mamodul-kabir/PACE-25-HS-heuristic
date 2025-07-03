import pandas as pd
import joblib
import numpy as np
from glob import glob
from skl2onnx import convert_sklearn
from skl2onnx.common.data_types import FloatTensorType
import onnx
import os

# Find any valid CSV file in the tfile folder
csv_files = glob("../tfile/*.csv")
if not csv_files:
    raise FileNotFoundError("No CSV files found in ../tfile")

df = pd.read_csv(csv_files[0], header=None)
df.replace([np.inf, -np.inf], np.nan, inplace=True)
df.dropna(inplace=True)
X = df.iloc[:, :-1]
X = np.clip(X, -1e6, 1e6)
n_features = X.shape[1]

# Load the model
model = joblib.load("rf_model.pkl")

# Convert to ONNX
initial_type = [('float_input', FloatTensorType([None, n_features]))]
onnx_model = convert_sklearn(
    model,
    initial_types=initial_type,
    options={id(model): {"zipmap": False}}
)

# Save ONNX model
onnx.save_model(onnx_model, "../rf_model.onnx")
print("ONNX model saved successfully as '../rf_model.onnx'")
