from enum import Enum
from typing import List, NamedTuple

import numpy as np
import odrive.enums
import pandas as pd


class ODriveModule(Enum):
    AXIS = 0
    MOTOR = 1
    SENSORLESS_ESTIMATOR = 2
    ENCODER = 3
    CONTROLLER = 4


class ODriveError(NamedTuple):
    timestamp: float
    module: ODriveModule
    num: int
    names: List[str]


module_name_map = {
    ODriveModule.AXIS: "AXIS",
    ODriveModule.MOTOR: "MOTOR",
    ODriveModule.ENCODER: "ENCODER",
    ODriveModule.CONTROLLER: "CONTROLLER",
    ODriveModule.SENSORLESS_ESTIMATOR: "SENSORLESS_ESTIMATOR",
}

error_col_map = {
    ODriveModule.AXIS: "axis_error",
    ODriveModule.MOTOR: "motor_error",
    ODriveModule.ENCODER: "encoder_error",
    ODriveModule.CONTROLLER: "controller_errror",
    ODriveModule.SENSORLESS_ESTIMATOR: "sensorless_estimator_error",
}

error_name_maps = {}
for odrive_module in ODriveModule:
    error_name_maps[odrive_module] = {
        v: k
        for k, v in odrive.enums.__dict__.items()
        if k.startswith(f"{module_name_map[odrive_module]}_ERROR")
    }


def getODriveErrorNames(odrive_module: ODriveModule, error_num: int) -> List[str]:
    error_names = []
    for bit in range(64):
        if int(error_num) & 1 << bit != 0:
            error_names.append(error_name_maps[odrive_module].get(1 << bit))
    return error_names


def getODriveErrors(df: pd.DataFrame) -> List[ODriveError]:
    errors = []
    for odrive_module in ODriveModule:
        error_col = error_col_map[odrive_module]
        if not error_col in df.columns:
            continue
        error_data = df[error_col]
        for idx in np.where(error_data != error_data.shift())[0]:
            error_num = error_data[idx]
            if error_num == 0:
                # TODO is skipping here okay
                continue
            error = ODriveError(
                timestamp=df["control_cycle_start_s"][idx],
                module=odrive_module,
                num=error_num,
                names=getODriveErrorNames(odrive_module, error_num),
            )
            errors.append(error)
    sorted_errors = sorted(
        errors, key=lambda error: (error.timestamp, error.module.value)
    )
    return sorted_errors
