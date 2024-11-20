<template>
  <div class="chart-container">
    <div v-if="debugInfo" class="debug-info">
      {{ debugInfo }}
    </div>
    <div ref="plotContainer" class="chart"></div>
    <div v-if="error" class="error-message">{{ error }}</div>
  </div>
</template>

<script>
import Plotly from 'plotly.js-dist';

export default {
  name: 'ChartsPage',
  data() {
    return {
      plotData: [],
      updateInterval: null,
      error: null,
      chartInitialized: false,
      debugInfo: null
    };
  },
  mounted() {
    console.log('ChartsPage mounted');
    this.initializeChart();
  },
  beforeUnmount() {
    if (this.updateInterval) {
      clearInterval(this.updateInterval);
    }
  },
  methods: {
    initializeChart() {
      this.fetchAndPlotData();
      this.updateInterval = setInterval(() => {
        this.fetchAndPlotData();
      }, 60000);
    },

    async fetchAndPlotData() {
      try {
        const response = await fetch('http://192.168.1.25:8000/sensor_data');
        if (!response.ok) throw new Error('Failed to fetch data');
        
        const rawData = await response.json();
        
        console.log('First data point:', rawData[0]);
        this.debugInfo = `Total data points: ${rawData.length}`;

        if (rawData.length === 0) {
          throw new Error('No data received');
        }

        const allProgramEvents = rawData.filter(row => row.Event_Type === 'program_event');
        console.log('All program events:', allProgramEvents);

        let fermentationData = [];
        let foundFermentation = false;

        for (let i = rawData.length - 1; i >= 0; i--) {
          const row = rawData[i];
          
          if (!foundFermentation && row.currentProgram === 'Fermentation') {
            foundFermentation = true;
            fermentationData.unshift(row);
          } else if (foundFermentation && row.currentProgram === 'Fermentation') {
            fermentationData.unshift(row);
          } else if (foundFermentation && row.currentProgram !== 'Fermentation') {
            break;
          }
        }

        console.log('Fermentation data found:', fermentationData);
        this.debugInfo += `\nFermentation points: ${fermentationData.length}`;

        if (fermentationData.length > 0) {
          const plotData = fermentationData.map(row => ({
            time: row.Backend_Time,
            waterTemp: this.parseNumericValue(row.waterTemp),
            pH: this.parseNumericValue(row.pH),
            oxygen: this.parseNumericValue(row.oxygen),
            turbidity: this.parseNumericValue(row.turbidity)
          })).filter(row => 
            !isNaN(row.waterTemp) || 
            !isNaN(row.pH) || 
            !isNaN(row.oxygen) || 
            !isNaN(row.turbidity)
          );

          console.log('Processed plot data:', plotData);
          this.debugInfo += `\nValid plot points: ${plotData.length}`;

          const traces = [
            {
              x: plotData.map(row => row.time),
              y: plotData.map(row => row.waterTemp),
              name: 'Temperature',
              line: { color: '#FF6B6B', width: 2 }
            },
            {
              x: plotData.map(row => row.time),
              y: plotData.map(row => row.pH),
              name: 'pH',
              xaxis: 'x2',
              yaxis: 'y2',
              line: { color: '#4DABF7', width: 2 }
            },
            {
              x: plotData.map(row => row.time),
              y: plotData.map(row => row.oxygen),
              name: 'Oxygen',
              xaxis: 'x3',
              yaxis: 'y3',
              line: { color: '#69DB7C', width: 2 }
            },
            {
              x: plotData.map(row => row.time),
              y: plotData.map(row => row.turbidity),
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
            yaxis: { 
              title: {
                text: 'Temperature (°C)',
                font: { color: '#E0E0E0' }
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
                font: { color: '#E0E0E0' }
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
                font: { color: '#E0E0E0' }
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
                font: { color: '#E0E0E0' }
              },
              showgrid: true, 
              gridwidth: 1, 
              gridcolor: '#2D2D2D',
              tickfont: { color: '#E0E0E0' },
              zeroline: false
            }
          };

          try {
            await Plotly.newPlot(this.$refs.plotContainer, traces, layout, {
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
              displaylogo: false
            });
            this.chartInitialized = true;
            console.log('Plot created successfully');
          } catch (plotError) {
            console.error('Error creating plot:', plotError);
            this.error = `Failed to create plot: ${plotError.message}`;
          }
        } else {
          this.error = "No fermentation data found";
        }

      } catch (error) {
        console.error('Error in fetchAndPlotData:', error);
        this.error = `Error: ${error.message}`;
      }
    },

    parseNumericValue(value) {
      if (value === null || value === undefined || value === '') return NaN;
      const parsed = parseFloat(value);
      return isNaN(parsed) || parsed === -1000 ? NaN : parsed;
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

.error-message {
  color: #FF6B6B;
  padding: 1rem;
  text-align: center;
  background-color: rgba(255, 107, 107, 0.1);
  border-radius: 8px;
  margin-top: 1rem;
}

.debug-info {
  position: fixed;
  top: 4rem;
  left: 1rem;
  background-color: rgba(0, 0, 0, 0.8);
  color: #fff;
  padding: 1rem;
  border-radius: 8px;
  font-family: monospace;
  white-space: pre-wrap;
  z-index: 1000;
  font-size: 0.875rem;
  max-width: 300px;
  opacity: 0.8;
  transition: opacity 0.2s ease;
}

.debug-info:hover {
  opacity: 1;
}

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