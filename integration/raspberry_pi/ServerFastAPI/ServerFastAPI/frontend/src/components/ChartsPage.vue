<template>
  <div class="container">
    <div id="chart" class="chart"></div>
  </div>
</template>

<script>
import Plotly from 'plotly.js-dist';

export default {
  name: 'ChartsPage',
  mounted() {
    this.fetchData();
  },
  methods: {
    async fetchData() {
      try {
        const response = await fetch('http://192.168.1.25:8000/sensor_data');
        const data = await response.json();
        if (data && data.length > 0) {
          this.plotData(data);
        } else {
          console.error("No data or invalid data received");
        }
      } catch (error) {
        console.error('Error fetching data:', error);
      }
    },
    plotData(data) {
      const trace1 = {
        x: data.map(d => d.Backend_Time),
        y: data.map(d => d.turbidity),
        name: 'Turbidity',
        type: 'scatter'
      };
      const trace2 = {
        x: data.map(d => d.Backend_Time),
        y: data.map(d => d.ph),
        name: 'pH',
        type: 'scatter',
        xaxis: 'x2',
        yaxis: 'y2'
      };
      const trace3 = {
        x: data.map(d => d.Backend_Time),
        y: data.map(d => d.oxygen),
        name: 'Oxygen',
        type: 'scatter',
        xaxis: 'x3',
        yaxis: 'y3'
      };
      const trace4 = {
        x: data.map(d => d.Backend_Time),
        y: data.map(d => d.waterTemp),
        name: 'Water Temperature',
        type: 'scatter',
        xaxis: 'x4',
        yaxis: 'y4'
      };
      const trace5 = {
        x: data.map(d => d.Backend_Time),
        y: data.map(d => d.airTemp),
        name: 'Air Temperature',
        type: 'scatter',
        xaxis: 'x4',
        yaxis: 'y4'
      };

      const dataToPlot = [trace1, trace2, trace3, trace4, trace5];

      const layout = {
        grid: {rows: 4, columns: 1, pattern: 'independent'},
        yaxis: {title: 'Turbidity', automargin: true},
        yaxis2: {title: 'pH', automargin: true},
        yaxis3: {title: 'Oxygen', automargin: true},
        yaxis4: {title: 'Temperature', automargin: true},
        xaxis4: {title: 'Time'},
        height: (window.innerHeight - 50) * 0.8,  // Adjust the height dynamically (reduced by 20%)
        margin: {l: 80, r: 80, t: 50, b: 80},  // Adjust margins for left, right, top, and bottom
        showlegend: true,
        width: window.innerWidth - 160  // Adjust the width dynamically to account for margins
      };

      // Update xaxes to hide labels for all but the last subplot
      layout.xaxis = {showticklabels: false};
      layout.xaxis2 = {showticklabels: false};
      layout.xaxis3 = {showticklabels: false};

      Plotly.newPlot('chart', dataToPlot, layout);

      // Resize the plot when the window is resized
      window.onresize = () => {
        Plotly.Plots.resize(document.getElementById('chart'));
      };
    }
  }
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

.chart {
  flex: 1 1 auto;
}
</style>