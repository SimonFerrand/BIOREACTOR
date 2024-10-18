<template>
  <div class="container">
    <div class="controls">
      <label for="start-time">Start Time:</label>
      <input type="datetime-local" id="start-time" v-model="startTime" />
      <label for="end-time">End Time:</label>
      <input type="datetime-local" id="end-time" v-model="endTime" />
      <button @click="fetchData">Load Data</button>
    </div>
    <div id="chart" class="chart"></div>
  </div>
</template>

<script>
import Plotly from 'plotly.js-dist';

export default {
  name: 'ChartsPage',
  data() {
    return {
      startTime: this.getDefaultStartTime(),
      endTime: this.getDefaultEndTime(),
      data: null,
    };
  },
  mounted() {
    this.fetchData();
  },
  methods: {
    getDefaultStartTime() {
      const now = new Date();
      now.setHours(now.getHours() - 48);
      return now.toISOString().slice(0, 16);
    },
    getDefaultEndTime() {
      const now = new Date();
      return now.toISOString().slice(0, 16);
    },
    async fetchData() {
      try {
        const response = await fetch(
          `http://192.168.1.25:8000/sensor_data?startTime=${this.startTime}&endTime=${this.endTime}`
        );
        if (response.ok) {
          this.data = await response.json();
          console.log('Raw data:', this.data);
          this.plotData();
        } else {
          console.error(`Error fetching data: ${response.status} - ${response.statusText}`);
        }
      } catch (error) {
        console.error('Error fetching data:', error);
      }
    },
    plotData() {
      if (!this.data || this.data.length === 0) {
        console.error("No data or invalid data received");
        return;
      }

      console.log('Raw data:', this.data);

      const parsedData = this.data.map(row => {
        console.log('Parsing row:', row);
        return {
          Backend_Time: row.Backend_Time,
          turbidity: parseFloat(row.turbidity),
          ph: parseFloat(row.ph),
          oxygen: parseFloat(row.oxygen),
          waterTemp: parseFloat(row.waterTemp),
          airTemp: parseFloat(row.airTemp),
          temperatureX: parseFloat(row.temperatureX),
          temperatureY: parseFloat(row.temperatureY),
        };
      });

      console.log('Parsed data:', parsedData);

      const trace1 = {
        x: parsedData.map(d => d.Backend_Time),
        y: parsedData.map(d => d.turbidity),
        name: 'Turbidity',
        type: 'scatter',
      };
      const trace2 = {
        x: parsedData.map(d => d.Backend_Time),
        y: parsedData.map(d => d.ph),
        name: 'pH',
        type: 'scatter',
        xaxis: 'x2',
        yaxis: 'y2',
      };
      const trace3 = {
        x: parsedData.map(d => d.Backend_Time),
        y: parsedData.map(d => d.oxygen),
        name: 'Oxygen',
        type: 'scatter',
        xaxis: 'x3',
        yaxis: 'y3',
      };
      const trace4 = {
        x: parsedData.map(d => d.Backend_Time),
        y: parsedData.map(d => d.waterTemp),
        name: 'Water Temperature',
        type: 'scatter',
        xaxis: 'x4',
        yaxis: 'y4',
      };
      const trace5 = {
        x: parsedData.map(d => d.Backend_Time),
        y: parsedData.map(d => d.airTemp),
        name: 'Air Temperature',
        type: 'scatter',
        xaxis: 'x4',
        yaxis: 'y4',
      };

      const dataToPlot = [trace1, trace2, trace3, trace4, trace5];

      const layout = {
        grid: { rows: 4, columns: 1, pattern: 'independent' },
        yaxis: { title: 'Turbidity', automargin: true },
        yaxis2: { title: 'pH', automargin: true },
        yaxis3: { title: 'Oxygen', automargin: true },
        yaxis4: { title: 'Temperature', automargin: true },
        xaxis4: { title: 'Time' },
        height: (window.innerHeight - 50) * 0.8, // Adjust the height dynamically (reduced by 20%)
        margin: { l: 80, r: 80, t: 50, b: 80 }, // Adjust margins for left, right, top, and bottom
        showlegend: true,
        width: window.innerWidth - 160, // Adjust the width dynamically to account for margins
      };

      // Update xaxes to hide labels for all but the last subplot
      layout.xaxis = { showticklabels: false };
      layout.xaxis2 = { showticklabels: false };
      layout.xaxis3 = { showticklabels: false };

      Plotly.newPlot('chart', dataToPlot, layout);

      // Resize the plot when the window is resized
      window.onresize = () => {
        Plotly.Plots.resize(document.getElementById('chart'));
      };
    },
  },
};
</script>

<style scoped>
.container {
  display: flex;
  flex-direction: column;
  height: 100vh;
  width: 100vw;
  overflow: hidden; /* Prevent overflow */
}

.controls {
  padding: 1rem;
  background-color: #f1f1f1;
  display: flex;
  justify-content: center;
  align-items: center;
  gap: 1rem;
}

.chart {
  flex: 1 1 auto;
}
</style>