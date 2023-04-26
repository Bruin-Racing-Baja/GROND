import os

import numpy as np
import pandas as pd
from google.protobuf import json_format

import header_message_pb2
import log_message_pb2
from car_constants import *

header_messag_id = 0
log_message_id = 1


class DescribedDataframe(pd.DataFrame):
    _metadata = ["filename", "description"]

    @property
    def _constructor(self):
        return DescribedDataframe


csv_cols = [
    "control_cycle_dt_us",
    "voltage",
    "last_heartbeat_ms",
    "wheel_rpm",
    "engine_rpm",
    "target_rpm",
    "velocity_command",
    "unclamped_velocity_command",
    "shadow_count",
    "inbound_estop",
    "outbound_estop",
    "iq_measured",
    "flushed",
    "wheel_count",
    "engine_count",
    "iq_setpoint",
    "control_cycle_start_us",
    "control_cycle_stop_us",
    "odrive_current",
    "axis_error",
    "motor_error",
    "encoder_error",
]
csv_cols_no_unclamp = [col for col in csv_cols if col != "unclamped_velocity_command"]


def getLogsByExtension(log_dir, ext):
    log_dir_contents = os.listdir(log_dir)
    txt_paths = []
    for potential_file in log_dir_contents:
        potential_file_path = os.path.join(log_dir, potential_file)
        potential_file_ext = os.path.splitext(potential_file)[1]
        if os.path.isfile(potential_file_path) and potential_file_ext == f".{ext}":
            txt_paths.append(potential_file_path)
    txt_paths.sort()
    return txt_paths


def filterFilesBySize(paths, size_kb):
    return [path for path in paths if os.path.getsize(path) >= size_kb * 1e3]


def convertCSVToBinary(path):
    df = pd.read_csv(path, skiprows=1, header=None)
    if len(df.columns) == 21:
        df.columns = csv_cols_no_unclamp
    else:
        df.columns = csv_cols
    df["inbound_estop"] = df["inbound_estop"].astype(bool)
    df["outbound_estop"] = df["outbound_estop"].astype(bool)
    df = df.drop("flushed", axis=1)

    path_without_ext = os.path.splitext(path)[0]
    with open(f"{path_without_ext}.bin", "wb") as bin_file:
        rows = df.to_dict(orient="records")
        log_message = log_message_pb2.LogMessage()
        for row in rows:
            json_format.ParseDict(row, log_message)
            serialized_log_message = log_message.SerializeToString()

            message_type = log_message_id
            message_length = len(serialized_log_message)
            delimiter = f"{message_type:01X}{message_length:04X}"

            bin_file.write(delimiter.encode())
            bin_file.write(serialized_log_message)


def parseCSVFile(path):
    try:
        df = pd.read_csv(path, skiprows=1, header=None, names=csv_cols)
    except:
        return None, None
    return None, DescribedDataframe(df)


def parseBinaryFile(path):
    header_message = None
    df = None
    with open(path, "rb") as file:
        all_rows = []
        header_message = header_message_pb2.HeaderMessage()
        while True:
            message_type_raw = file.read(1)
            if message_type_raw == b"":
                break
            message_length_raw = file.read(4)

            message_type = int(message_type_raw, 16)
            message_length = int(message_length_raw, 16)
            message = file.read(message_length)

            if message_type == header_messag_id:
                header_message.ParseFromString(message)
            elif message_type == log_message_id:
                message_type = log_message_pb2.LogMessage.DESCRIPTOR
                log_message = log_message_pb2.LogMessage()
                log_message.ParseFromString(message)
                row_values = []
                for field in log_message_pb2.LogMessage.DESCRIPTOR.fields:
                    row_values.append(getattr(log_message, field.name))
                all_rows.append(row_values)
        columns = [field.name for field in log_message_pb2.LogMessage.DESCRIPTOR.fields]
        df = DescribedDataframe(all_rows, columns=columns)
    return header_message, df


def addNormalizedColumns(df):
    for col in df:
        col_norm = f"norm_{col}"
        if col.startswith("norm_") or col_norm in df:
            continue
        col_obj = df[col]
        if np.issubdtype(col_obj.dtype, np.number):
            if np.max(col_obj) == np.min(col_obj):
                df[col_norm] = 0.0
            else:
                df[col_norm] = (col_obj - np.min(col_obj)) / (
                    np.max(col_obj) - np.min(col_obj)
                )


def postProcessDataframe(df):
    df["control_cycle_start_s"] = df["control_cycle_start_us"] / 1e6
    df["control_cycle_dt_s"] = df["control_cycle_dt_us"] / 1e6

    df["secondary_rpm"] = df["wheel_rpm"] * (57 / 18) * (45 / 17)
    df["wheel_mph"] = (df["wheel_rpm"] * wheel_diameter * np.pi) / (12 * 5280) * 60

    df["vehicle_position_feet"] = (
        np.cumsum(df["wheel_mph"] * 5280 * df["control_cycle_dt_s"]) / 3600
    )

    df["actuator_position_mm"] = -df["shadow_count"] / encoder_cpr * pitch_angle

    df["shift_ratio"] = df["secondary_rpm"] / df["engine_rpm"]
    df["shift_ratio"] = df["shift_ratio"].clip(lower=0.2, upper=2)

    addNormalizedColumns(df)


def trimDataFrame(df, start_s=0, end_s=float("inf")):
    df = df.loc[
        (start_s < df["control_cycle_start_s"]) & (df["control_cycle_start_s"] < end_s)
    ]
    return df
