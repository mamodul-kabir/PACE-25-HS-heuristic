import pandas as pd
import os
import numpy as np
from glob import glob
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.metrics import classification_report
import joblib
import sys
import gc
import psutil

from skl2onnx import convert_sklearn
from skl2onnx.common.data_types import FloatTensorType
import onnx

# Step 1: Load and clean data
folder_path = "../training"  # Replace with your actual path
csv_files = glob(os.path.join(folder_path, "*.csv"))

cleaned_dataframes = []
for file in csv_files:
    df = pd.read_csv(file, header=None)
    df.replace([np.inf, -np.inf], np.nan, inplace=True)
    df.dropna(inplace=True)
    cleaned_dataframes.append(df)

data = pd.concat(cleaned_dataframes, ignore_index=True)

# Step 2: Prepare features and labels
X = data.iloc[:, :-1]
y = data.iloc[:, -1]

X = np.clip(X, -1e6, 1e6)

# Step 3: Train/test split
X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42
)

# Step 4: Train model
model = RandomForestClassifier(
    n_estimators=26,
    class_weight='balanced',
    n_jobs=-1,
    random_state=42
)
model.fit(X_train, y_train)

# Step 5: Clean X_test (safety)
X_test.replace([np.inf, -np.inf], np.nan, inplace=True)
test_nan_mask = X_test.isna().any(axis=1)
if test_nan_mask.any():
    print(f"Dropping {test_nan_mask.sum()} rows with NaN/inf in X_test.")
    X_test = X_test[~test_nan_mask]
    y_test = y_test[~test_nan_mask]

X_test = np.clip(X_test, -1e6, 1e6)

# Step 6: Evaluate
y_pred = model.predict(X_test)
print("Classification Report:")
print(classification_report(y_test, y_pred))

# Step 7: Feature importances
print("Feature Importances:")
for idx, val in enumerate(model.feature_importances_):
    print(f"Feature {idx}: {val:.4f}")

print("Memory used (MB):", psutil.Process(os.getpid()).memory_info().rss / 1024 / 1024)

gc.collect()

# Step 8: Save using ONNX with full class probabilities
initial_type = [('float_input', FloatTensorType([None, X.shape[1]]))]
onnx_model = convert_sklearn(
    model,
    initial_types=initial_type,
    options={id(model): {"zipmap": False}}  # ensures probabilities are output as raw array
)
onnx.save_model(onnx_model, "../random_forest_model.onnx")
print("ONNX model saved successfully as 'random_forest_model.onnx'!", file=sys.stderr)
