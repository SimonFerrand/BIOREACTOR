import pandas as pd
import plotly.graph_objects as go
from plotly.subplots import make_subplots
from datetime import datetime

# Read CSV file
df = pd.read_csv("R:/Bioreactor/ServerFastAPI/data/data.csv")

# Convert Backend_Time column to datetime
df['Backend_Time'] = pd.to_datetime(df['Backend_Time'])

# Find the index of the last program_event
last_program_event = df[df['Event_Type'] == 'program_event'].index.max()

if pd.notna(last_program_event):
    # Take all data after the last program_event where currentProgram is 'Fermentation'.
    plot_data = df[last_program_event:][df['currentProgram'] == 'Fermentation'].copy()
    
    if not plot_data.empty:
        # Create a figure with 4 subgraphs
        fig = make_subplots(
            rows=4, 
            cols=1,
            shared_xaxes=True,
            vertical_spacing=0.08,
            subplot_titles=('Water Temperature (°C)', 'pH', 'Dissolved Oxygen (%)', 'Turbidity (NTU)')
        )

        # Trace configuration
        fig.add_trace(
            go.Scatter(
                x=plot_data['Backend_Time'],
                y=plot_data['waterTemp'],
                name='Temperature',
                line=dict(color='red', width=2)
            ),
            row=1, col=1
        )

        fig.add_trace(
            go.Scatter(
                x=plot_data['Backend_Time'],
                y=plot_data['pH'],
                name='pH',
                line=dict(color='blue', width=2)
            ),
            row=2, col=1
        )

        fig.add_trace(
            go.Scatter(
                x=plot_data['Backend_Time'],
                y=plot_data['oxygen'],
                name='Oxygen',
                line=dict(color='green', width=2)
            ),
            row=3, col=1
        )

        fig.add_trace(
            go.Scatter(
                x=plot_data['Backend_Time'],
                y=plot_data['turbidity'],
                name='Turbidity',
                line=dict(color='purple', width=2)
            ),
            row=4, col=1
        )

        # Update layout
        fig.update_layout(
            height=1000,
            showlegend=True,
            title={
                'text': 'Bioreactor Parameters Monitoring',
                'y':0.95,
                'x':0.5,
                'xanchor': 'center',
                'yanchor': 'top',
                'font': dict(size=24)
            },
            legend=dict(
                orientation="h",
                yanchor="bottom",
                y=1.02,
                xanchor="right",
                x=1
            ),
            margin=dict(t=100)
        )

        # Update axes
        fig.update_xaxes(showgrid=True, gridwidth=1, gridcolor='LightGray')
        fig.update_yaxes(showgrid=True, gridwidth=1, gridcolor='LightGray')
        
        # Specific to each subgraph
        fig.update_yaxes(title_text="Temperature (°C)", row=1, col=1)
        fig.update_yaxes(title_text="pH", row=2, col=1)
        fig.update_yaxes(title_text="DO (%)", row=3, col=1)
        fig.update_yaxes(title_text="Turbidity (NTU)", row=4, col=1)
        
        # Add an xlabel only on the last chart
        fig.update_xaxes(title_text="Time", row=4, col=1)

        # Display figure
        fig.show()
    else:
        print("No fermentation data after last program event")
else:
    print("No fermentation program events found")