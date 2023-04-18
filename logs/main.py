#!/usr/bin/env python
import os
import sys
import log_message_pb2
import header_message_pb2


def decode_proto(file_path):
    try:
        with open(file_path, "rb") as f:
            while True:
                message_type_raw = f.read(1)
                if message_type_raw == b"":
                    break
                message_length_raw = f.read(4)
                message_type = int(message_type_raw, 16)
                message_length = int(message_length_raw, 16)
                message = f.read(message_length)

                if message_type == 0:
                    header_message = header_message_pb2.HeaderMessage()
                    parsed_size = header_message.ParseFromString(message)
                    print(
                        f"{header_message.timestamp_human}, {header_message.clock_us}, {header_message.p_gain}"
                    )
                elif message_type == 1:
                    log_message = log_message_pb2.LogMessage()
                    parsed_size = log_message.ParseFromString(message)
                    print(f"{log_message.control_cycle_count},", end="")
                    print(f"{log_message.control_cycle_start_us},", end="")
                    print(f"{log_message.control_cycle_stop_us},", end="")
                    print(f"{log_message.control_cycle_dt_us},", end="")
                    print(f"{log_message.control_cycle_dt_us},", end="")
                    print(f"{log_message.wheel_rpm},", end="")
                    print(f"{log_message.engine_rpm},", end="")
                    print(f"{log_message.engine_count},", end="")
                    print(f"{log_message.wheel_count},", end="")
                    print(f"{log_message.target_rpm},", end="")
                    print(f"{log_message.velocity_command},", end="")
                    print(f"{log_message.last_heartbeat_ms},", end="")
                    print(f"{log_message.axis_error},", end="")
                    print(f"{log_message.motor_error},", end="")
                    print(f"{log_message.encoder_error},", end="")
                    print(f"{log_message.voltage},", end="")
                    print(f"{log_message.iq_measured},", end="")
                    print(f"{log_message.iq_setpoint},", end="")
                    print(f"{log_message.odrive_current},", end="")
                    print(f"{log_message.inbound_estop},", end="")
                    print(f"{log_message.outbound_estop},", end="")
                    print(f"{log_message.shadow_count},", end="")
                    print("")

    except FileNotFoundError:
        print("File not found:", file_path)


path = "/run/media/grant/CHEESE"
files = os.listdir(path)
logs = [f for f in files if os.path.isfile(os.path.join(path, f))]
logs.sort()
latest_log = os.path.join(path, logs[-1])

print(f"{latest_log}:")

decode_proto(latest_log)
