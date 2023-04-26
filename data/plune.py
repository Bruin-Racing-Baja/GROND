#!/usr/bin/env python
import argparse
import os
from typing import List, NamedTuple

import plotly.graph_objects as go
from dash import Dash, Input, Output, callback, dcc, html, no_update

import log_parser
from figures import figuresToHTML


class FigureInfo(NamedTuple):
    x_axis: str
    y_axis: List[str]
    title: str


paths = log_parser.getLogsByExtension("logs", "bin")

figure_infos = [
    FigureInfo(
        x_axis="control_cycle_start_s",
        y_axis=["engine_rpm", "secondary_rpm", "target_rpm"],
        title="Engine, Secondary, Target RPM",
    ),
    FigureInfo(
        x_axis="control_cycle_start_s",
        y_axis=["norm_engine_rpm", "norm_secondary_rpm", "norm_actuator_position_mm"],
        title="Normalized Engine/Secondary RPM, Actuator Position",
    ),
    FigureInfo(
        x_axis="control_cycle_start_s",
        y_axis=["norm_shift_ratio", "norm_actuator_position_mm"],
        title="Normalized Shift Ratio, Actuator Position",
    ),
    FigureInfo(
        x_axis="control_cycle_start_s",
        y_axis=["velocity_command", "unclamped_velocity_command"],
        title="Velocity Command",
    ),
    FigureInfo(
        x_axis="control_cycle_start_s", y_axis=["shadow_count"], title="Shadow Count"
    ),
    FigureInfo(
        x_axis="control_cycle_start_s", y_axis=["wheel_mph"], title="Vehciel Speed"
    ),
    FigureInfo(
        x_axis="control_cycle_start_s",
        y_axis=["vehicle_position_feet"],
        title="Vehcile Position",
    ),
]

app = Dash(__name__)


app.layout = html.Div(
    [
        html.H1(children="Loading...", style={"textAlign": "center"}, id="title"),
        html.H4(children="Loading...", style={"textAlign": "center"}, id="subtitle"),
        dcc.Dropdown(paths, paths[-1], id="file-selection"),
        html.Div([], id="graphs"),
    ]
)


@callback(
    [
        Output("title", "children"),
        Output("subtitle", "children"),
        Output("graphs", "children"),
        Input("file-selection", "value"),
    ]
)
def onFileSelection(path):
    if path == None:
        return no_update

    header, df = log_parser.parseBinaryFile(path)
    log_parser.postProcessDataframe(df)

    title = header.timestamp_human
    if title == "":
        title = "Timestamp Missing"

    graphs = [dcc.Graph() for _ in range(len(figure_infos))]

    for i, figure_info in enumerate(figure_infos):
        fig = go.Figure()
        traces = [
            go.Scatter(x=df[figure_info.x_axis], y=df[y_axis], name=y_axis)
            for y_axis in figure_info.y_axis
        ]
        fig.add_traces(traces)
        fig.update_layout(
            title=figure_info.title,
            xaxis_title=figure_info.x_axis,
            showlegend=True,
            margin=dict(l=20, r=20, t=60, b=20),
        )
        graphs[i].figure = fig

    return title, path, graphs


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Script description")

    parser.add_argument(
        "-e", "--export", action="store_true", help="Export all logs to html graphs"
    )

    args = parser.parse_args()

    if args.export:
        for path in paths:
            print
            header, df = log_parser.parseBinaryFile(path)
            log_parser.postProcessDataframe(df)
            figures = []
            for i, figure_info in enumerate(figure_infos):
                figure = go.Figure()
                traces = [
                    go.Scatter(x=df[figure_info.x_axis], y=df[y_axis], name=y_axis)
                    for y_axis in figure_info.y_axis
                ]
                figure.add_traces(traces)
                figure.update_layout(
                    title=figure_info.title,
                    xaxis_title=figure_info.x_axis,
                    showlegend=True,
                )
                figures.append(figure)

            filename_without_ext = os.path.splitext(os.path.basename(path))[0]
            html_path = f"graphs/{filename_without_ext}.html"
            print(f"Exporting {path} -> {html_path}")
            figuresToHTML(figures, html_path)
    else:
        app.run_server(debug=True)
