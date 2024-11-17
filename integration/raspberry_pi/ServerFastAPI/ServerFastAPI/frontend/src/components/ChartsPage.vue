<template>
  <div class="chart-container">
    <div ref="plotContainer" class="chart"></div>
  </div>
</template>

<script>
import Plotly from 'plotly.js-dist';

export default {
  name: 'ChartsPage',
  data() {
    return {
      plotData: [],
      updateInterval: null
    };
  },
  mounted() {
    this.initializeChart();
    window.addEventListener('resize', this.handleResize);
  },
  beforeUnmount() {
    if (this.updateInterval) {
      clearInterval(this.updateInterval);
    }
    window.removeEventListener('resize', this.handleResize);
  },
  methods: {
    handleResize() {
      const update = {
        height: window.innerHeight - 120, // 120px pour la navbar et les marges
        width: window.innerWidth - 48     // 48px pour les marges latérales
      };
      Plotly.relayout(this.$refs.plotContainer, update);
    },
    
    initializeChart() {
      this.fetchAndPlotData();
      this.updateInterval = setInterval(() => {
        this.fetchAndPlotData();
      }, 60000); // Actualisation toutes les minutes
    },

    async fetchAndPlotData() {
      try {
        const response = await fetch('http://192.168.1.25:8000/sensor_data');
        if (!response.ok) throw new Error('Failed to fetch data');
        
        const data = await response.json();
        
        // Trouver le dernier program_event
        const programEvents = data.filter(row => row.Event_Type === 'program_event');
        const lastProgramEvent = programEvents[programEvents.length - 1];

        if (lastProgramEvent) {
          const lastEventIndex = data.indexOf(lastProgramEvent);
          const plotData = data
            .slice(lastEventIndex)
            .filter(row => row.currentProgram === 'Fermentation');

          if (plotData.length > 0) {
            const traces = [
              {
                x: plotData.map(row => row.Backend_Time),
                y: plotData.map(row => parseFloat(row.waterTemp)),
                name: 'Temperature',
                line: { color: '#FF6B6B', width: 2 }
              },
              {
                x: plotData.map(row => row.Backend_Time),
                y: plotData.map(row => parseFloat(row.pH)),
                name: 'pH',
                xaxis: 'x2',
                yaxis: 'y2',
                line: { color: '#4DABF7', width: 2 }
              },
              {
                x: plotData.map(row => row.Backend_Time),
                y: plotData.map(row => parseFloat(row.oxygen)),
                name: 'Oxygen',
                xaxis: 'x3',
                yaxis: 'y3',
                line: { color: '#69DB7C', width: 2 }
              },
              {
                x: plotData.map(row => row.Backend_Time),
                y: plotData.map(row => parseFloat(row.turbidity)),
                name: 'Turbidity',
                xaxis: 'x4',
                yaxis: 'y4',
                line: { color: '#DA77F2', width: 2 }
              }
            ];

            const layout = {
              height: window.innerHeight - 120,
              width: window.innerWidth - 48,
              paper_bgcolor: '#121212',
              plot_bgcolor: '#121212',
              showlegend: true,
              grid: {
                rows: 4,
                columns: 1,
                pattern: 'independent',
                roworder: 'top to bottom',
                height: 0.8
              },
              title: {
                text: 'Bioreactor Parameters Monitoring',
                y: 0.98,
                x: 0.5,
                xanchor: 'center',
                yanchor: 'top',
                font: { 
                  size: 24,
                  color: '#E0E0E0',
                  family: 'Inter, system-ui, sans-serif'
                }
              },
              legend: {
                orientation: "h",
                yanchor: "bottom",
                y: 1.02,
                xanchor: "right",
                x: 1,
                font: { color: '#E0E0E0' },
                bgcolor: '#1E1E1E',
                bordercolor: '#2D2D2D'
              },
              margin: { 
                t: 50,
                l: 80,
                r: 50,
                b: 50,
                pad: 0
              },
              
              // Configuration des axes X
              xaxis: { 
                showgrid: true, 
                gridwidth: 1, 
                gridcolor: '#2D2D2D', 
                showticklabels: false,
                tickfont: { color: '#E0E0E0' },
                zeroline: false
              },
              xaxis2: { 
                showgrid: true, 
                gridwidth: 1, 
                gridcolor: '#2D2D2D', 
                showticklabels: false,
                tickfont: { color: '#E0E0E0' },
                zeroline: false
              },
              xaxis3: { 
                showgrid: true, 
                gridwidth: 1, 
                gridcolor: '#2D2D2D', 
                showticklabels: false,
                tickfont: { color: '#E0E0E0' },
                zeroline: false
              },
              xaxis4: { 
                showgrid: true, 
                gridwidth: 1, 
                gridcolor: '#2D2D2D', 
                title: {
                  text: 'Time',
                  font: { 
                    color: '#E0E0E0',
                    family: 'Inter, system-ui, sans-serif'
                  }
                },
                tickfont: { color: '#E0E0E0' },
                zeroline: false
              },
              
              // Configuration des axes Y
              yaxis: { 
                title: {
                  text: 'Temperature (°C)',
                  font: { 
                    color: '#E0E0E0',
                    family: 'Inter, system-ui, sans-serif'
                  }
                },
                showgrid: true, 
                gridwidth: 1, 
                gridcolor: '#2D2D2D',
                tickfont: { color: '#E0E0E0' },
                zeroline: false
              },
              yaxis2: { 
                title: {
                  text: 'pH',
                  font: { 
                    color: '#E0E0E0',
                    family: 'Inter, system-ui, sans-serif'
                  }
                },
                showgrid: true, 
                gridwidth: 1, 
                gridcolor: '#2D2D2D',
                tickfont: { color: '#E0E0E0' },
                zeroline: false
              },
              yaxis3: { 
                title: {
                  text: 'DO (%)',
                  font: { 
                    color: '#E0E0E0',
                    family: 'Inter, system-ui, sans-serif'
                  }
                },
                showgrid: true, 
                gridwidth: 1, 
                gridcolor: '#2D2D2D',
                tickfont: { color: '#E0E0E0' },
                zeroline: false
              },
              yaxis4: { 
                title: {
                  text: 'Turbidity (NTU)',
                  font: { 
                    color: '#E0E0E0',
                    family: 'Inter, system-ui, sans-serif'
                  }
                },
                showgrid: true, 
                gridwidth: 1, 
                gridcolor: '#2D2D2D',
                tickfont: { color: '#E0E0E0' },
                zeroline: false
              }
            };

            Plotly.newPlot(this.$refs.plotContainer, traces, layout, {
              responsive: true,
              displayModeBar: true,
              modeBarButtonsToRemove: [
                'zoom2d',
                'pan2d',
                'select2d',
                'lasso2d',
                'zoomIn2d',
                'zoomOut2d',
                'autoScale2d',
                'resetScale2d',
              ],
              displaylogo: false,
              toImageButtonOptions: {
                format: 'png',
                filename: 'bioreactor_data',
                height: 1080,
                width: 1920,
                scale: 2
              }
            });
          }
        }
      } catch (error) {
        console.error('Error fetching or plotting data:', error);
      }
    }
  }
};
</script>

<style scoped>
.chart-container {
  height: calc(100vh - 4rem);
  padding: 1rem;
  background-color: var(--dark-bg);
  overflow: hidden;
}

.chart {
  width: 100%;
  height: 100%;
  border-radius: 12px;
  overflow: hidden;
}

/* Styles pour les éléments Plotly */
:deep(.modebar) {
  background-color: #1E1E1E !important;
}

:deep(.modebar-btn) {
  color: #E0E0E0 !important;
}

:deep(.modebar-btn:hover) {
  color: #FFFFFF !important;
  background-color: #2D2D2D !important;
}
</style>