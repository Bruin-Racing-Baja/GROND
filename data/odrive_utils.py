from enum import Enum

import numpy as np
import odrive.enums


class ErrorType(Enum):
    AXIS = 0
    MOTOR = 1
    SENSORLESS_ESTIMATOR = 2
    ENCODER = 3
    CONTROLLER = 4


error_type_infos = {
    ErrorType.MOTOR: ("motor_error", "Motor Errors"),
    ErrorType.AXIS: ("axis_error", "Axis Errors"),
    ErrorType.ENCODER: ("encoder_error", "Encoder Errors"),
}


def printErrors(error, error_type, prefix=""):
    error = int(error)
    if error_type == ErrorType.AXIS:
        error_map_prefix = "AXIS_ERROR_"
    elif error_type == ErrorType.MOTOR:
        error_map_prefix = "MOTOR_ERROR_"
    elif error_type == ErrorType.SENSORLESS_ESTIMATOR:
        error_map_prefix = "SENSORLESS_ESTIMATOR_ERROR_"
    elif error_type == ErrorType.ENCODER:
        error_map_prefix = "ENCODER_ERROR_"
    elif error_type == ErrorType.CONTROLLER:
        error_map_prefix = "CONTROLLER_ERROR_"
    error_map = {
        v: k for k, v in odrive.enums.__dict__.items() if k.startswith(error_map_prefix)
    }
    for bit in range(64):
        if error & (1 << bit) != 0:
            print(f"{prefix}{error_map.get(1 << bit)}")


def printErrorTimes(df, error_type):
    error_type_info = error_type_infos[error_type]
    idxs = np.where(df[error_type_info[0]] != df[error_type_info[0]].shift())

    for idx in idxs[0]:
        error_time_s = df.iloc[idx]["control_cycle_start_s"]
        error = df.iloc[idx][error_type_info[0]]
        print(f"{error_type_info[1]} ({error_time_s:.02f}):")
        if error == 0:
            print("  NONE")
        else:
            printErrors(error, error_type, "  ")
