import pandas as pd
import plotly.graph_objects as go
from plotly.graph_objs import Figure


def getRPMFigure(df: pd.DataFrame) -> Figure:
    fig = go.Figure()
    fig.add_trace(
        go.Scatter(x=df["control_cycle_start_s"], y=df["engine_rpm"], name="Engine RPM")
    )
    fig.add_trace(
        go.Scatter(
            x=df["control_cycle_start_s"], y=df["secondary_rpm"], name="Secondary RPM"
        )
    )
    fig.add_trace(
        go.Scatter(x=df["control_cycle_start_s"], y=df["target_rpm"], name="Target RPM")
    )
    fig.update_layout(
        xaxis_title="Time (s)",
        yaxis_title="RPM",
        title=f"Engine RPM and Secondary RPM<br><sup>{df.filename}</sup>",
    )
    fig.update_traces(showlegend=True)
    return fig


def getRPMAndActuatorFigure(df: pd.DataFrame) -> Figure:
    fig = go.Figure()
    fig.add_trace(
        go.Scatter(
            x=df["control_cycle_start_s"], y=df["norm_engine_rpm"], name="Engine RPM"
        )
    )
    fig.add_trace(
        go.Scatter(
            x=df["control_cycle_start_s"],
            y=df["norm_secondary_rpm"],
            name="Secondary RPM",
        )
    )
    fig.add_trace(
        go.Scatter(
            x=df["control_cycle_start_s"],
            y=df["norm_actuator_position_mm"],
            name="Actuator Position",
        )
    )
    fig.update_layout(
        xaxis_title="Time (s)",
        title=f"Normalized Engine RPM, Secondary RPM, and Actuator Position<br><sup>{df.filename}</sup>",
    )
    fig.update_traces(showlegend=True)
    return fig


def getVehicleSpeedFigure(df: pd.DataFrame) -> Figure:
    fig = go.Figure()
    fig.add_trace(
        go.Scatter(
            x=df["control_cycle_start_s"],
            y=df["wheel_mph"],
        )
    )
    fig.update_layout(
        xaxis_title="Time (s)",
        yaxis_title="Vehicle Speed (mph)",
        title=f"Vehicle Speed<br><sup>{df.filename}</sup>",
    )
    return fig


def getShiftRatioAndAcuatorFigure(df: pd.DataFrame) -> Figure:
    fig = go.Figure()
    fig.add_trace(
        go.Scatter(
            x=df["control_cycle_start_s"],
            y=df["norm_actuator_position_mm"],
            name="Actuator Position (mm)",
        )
    )
    fig.add_trace(
        go.Scatter(
            x=df["control_cycle_start_s"], y=df["norm_shift_ratio"], name="Shift Ratio"
        )
    )
    fig.update_layout(
        xaxis_title="Time (s)",
        title=f"Normalized Actuator Position and Shift Ratio<br><sup>{df.filename}</sup>",
    )
    fig.update_traces(showlegend=True)
    return fig


def getVelocityCommandFigure(df: pd.DataFrame) -> Figure:
    fig = go.Figure()
    fig.add_trace(
        go.Scatter(
            x=df["control_cycle_start_s"],
            y=df["velocity_command"],
            name="Velocity Command",
        )
    )
    fig.add_trace(
        go.Scatter(
            x=df["control_cycle_start_s"],
            y=df["unclamped_velocity_command"],
            name="Unclamped Velocity Command",
        )
    )
    fig.update_layout(
        xaxis_title="Time (s)",
        yaxis_title="Velocity Command",
        title=f"Velocity Command<br><sup>{df.filename}</sup>",
    )
    return fig


def getShadowCountFigure(df: pd.DataFrame) -> Figure:
    fig = go.Figure()
    fig.add_trace(
        go.Scatter(
            x=df["control_cycle_start_s"], y=df["shadow_count"], name="Shadow Count"
        )
    )
    fig.update_layout(
        xaxis_title="Time (s)",
        yaxis_title="Shadow Count",
        title=f"Shadow Count<br><sup>{df.filename}</sup>",
    )
    return fig


def getShiftRatioFigure(df: pd.DataFrame) -> Figure:
    fig = go.Figure()
    fig.add_trace(
        go.Scatter(
            x=df["control_cycle_start_s"], y=df["shift_ratio"], name="Shift Ratio"
        )
    )
    fig.update_layout(
        xaxis_title="Time (s)",
        title=f"Shift Ratio<br><sup>{df.filename}</sup>",
    )
    return fig


def getEngineVsWheelFigure(df: pd.DataFrame) -> Figure:
    fig = go.Figure()
    fig.add_trace(
        go.Scatter(
            x=df["wheel_mph"], y=df["engine_rpm"], name="Engine RPM vs Wheel Speed"
        )
    )
    fig.update_layout(
        xaxis_title="Wheel Speed (mph)",
        yaxis_title="Engine RPM",
        title=f"Engine RPM vs Wheel Speed<br><sup>{df.filename}</sup>",
    )
    return fig


def getVoltageAndCurrentFigure(df: pd.DataFrame) -> Figure:
    fig = go.Figure()
    fig.add_trace(
        go.Scatter(x=df["control_cycle_start_s"], y=df["voltage"], name="Voltage")
    )
    fig.add_trace(
        go.Scatter(
            x=df["control_cycle_start_s"], y=df["iq_measured"], name="IQ Measured"
        )
    )
    fig.update_layout(
        xaxis_title="Time (s)",
        title=f"Current and Voltage<br><sup>{df.filename}</sup>",
    )
    return fig


def getEverythingFigure(df: pd.DataFrame) -> Figure:
    # TODO ensure columns exist
    fig = go.Figure()
    try:
        for col in df.columns:
            if col.startswith("norm"):
                fig.add_trace(
                    go.Scatter(x=df["control_cycle_start_s"], y=df[col], name=col)
                )
        fig.update_layout(
            title=f"Everything<br><sup>{df.filename}</sup>",
        )
    except KeyError:
        fig = go.Figure()
    return fig


def getFilteredSecondaryFigure(df: pd.DataFrame) -> Figure:
    try:
        fig = go.Figure()
        fig.add_trace(
            go.Scatter(
                x=df["control_cycle_start_s"],
                y=df["filtered_secondary_rpm"],
                name="Fitlered Secondary RPM",
            )
        )
        fig.add_trace(
            go.Scatter(
                x=df["control_cycle_start_s"],
                y=df["secondary_rpm"],
                name="Fitlered Secondary RPM",
            )
        )
        fig.update_layout(
            xaxis_title="Time (s)",
            title=f"Fitlered Secondary RPM<br><sup>{df.filename}</sup>",
        )
    except KeyError:
        pass
    return fig


def figuresToHTML(figs, filename, offline=False, dark_theme=False):
    with open(filename, "w") as file:
        background_color = "black" if dark_theme else "white"
        file.write(
            f"<html style='background-color: {background_color}'><head></head><body>\n"
        )
        for fig in figs:
            font_size_backup = fig.layout.font.size
            fig.update_layout(font={"size": 20})
            inner_html = (
                fig.to_html(include_plotlyjs=(True if offline else "cdn"))
                .split("<body>")[1]
                .split("</body>")[0]
            )
            file.write(inner_html)
            fig.update_layout(font={"size": font_size_backup})
        file.write("</body></html>\n")
