# train_rf_and_save.py
import pandas as pd
import os
import numpy as np
from glob import glob
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.metrics import classification_report
import joblib
import psutil

folder_path = "../tfile"
csv_files = glob(os.path.join(folder_path, "*.csv"))

instance_data = []
for file in csv_files:
    df = pd.read_csv(file, header=None)
    df.replace([np.inf, -np.inf], np.nan, inplace=True)
    df.dropna(inplace=True)
    X_i = df.iloc[:, :-1]
    y_i = df.iloc[:, -1]
    X_i = np.clip(X_i, -1e6, 1e6)
    instance_data.append((X_i, y_i))

train_data, test_data = train_test_split(instance_data, test_size=0.1, random_state=42)

X_train = pd.concat([X for X, _ in train_data], ignore_index=True)
y_train = pd.concat([y for _, y in train_data], ignore_index=True)
X_test = pd.concat([X for X, _ in test_data], ignore_index=True)
y_test = pd.concat([y for _, y in test_data], ignore_index=True)

# Clean test set
X_test.replace([np.inf, -np.inf], np.nan, inplace=True)
test_nan_mask = X_test.isna().any(axis=1)
if test_nan_mask.any():
    X_test = X_test[~test_nan_mask]
    y_test = y_test[~test_nan_mask]
X_test = np.clip(X_test, -1e6, 1e6)

# Train model
model = RandomForestClassifier(
    n_estimators=100,
    max_depth=20,
    min_samples_leaf=3,
    class_weight='balanced_subsample',
    n_jobs=3,
    random_state=42,
    verbose=1
)
model.fit(X_train, y_train)

# Evaluate
y_pred = model.predict(X_test)
print("Classification Report:")
print(classification_report(y_test, y_pred))

print("Feature Importances:")
for idx, val in enumerate(model.feature_importances_):
    print(f"Feature {idx}: {val:.4f}")

print("Memory used (MB):", psutil.Process(os.getpid()).memory_info().rss / 1024 / 1024)

# Save model
joblib.dump(model, "rf_model.pkl")
print("Model saved as 'rf_model.pkl'")
