<template>
  <div class="pid-container">
    <header>
      <h1>PID Control Dashboard</h1>
      <p>Monitor the bioreactor's current state and parameters.</p>
    </header>
    <section class="current-program">
      <h2>Current Program</h2>
      <div v-if="currentProgram">
        <p><strong>Program Name:</strong> {{ currentProgram.name }}</p>
        <p><strong>Status:</strong> {{ currentProgram.status }}</p>
        <ul>
          <li v-for="(value, key) in currentProgram.parameters" :key="key">
            <strong>{{ key }}:</strong> {{ value }}
          </li>
        </ul>
      </div>
      <div v-else>
        <p>No program is currently running.</p>
      </div>
    </section>
    <section class="latest-data">
      <h2>Latest Sensor Data</h2>
      <table>
        <thead>
          <tr>
            <th>Sensor</th>
            <th>Value</th>
          </tr>
        </thead>
        <tbody>
          <tr v-for="(value, key) in latestData" :key="key">
            <td>{{ key }}</td>
            <td>{{ value }}</td>
          </tr>
        </tbody>
      </table>
    </section>
    <section class="historical-data">
      <h2>Historical Data</h2>
      <p>Click to expand and view historical data.</p>
      <!-- Historical data table or chart goes here -->
    </section>
  </div>
</template>

<script>
import { ref, onMounted } from 'vue';
import { useWebSocket } from '@/composables/useWebSocket';

export default {
  name: 'PIDPage',
  setup() {
    const { lastMessage, connectionStatus } = useWebSocket('ws://192.168.1.25:8000/ws');
    const currentProgram = ref(null);
    const latestData = ref({});

    const fetchLatestData = async () => {
      try {
        const response = await fetch('http://192.168.1.25:8000/sensor_data');
        const data = await response.json();
        if (data && data.length > 0) {
          latestData.value = data[data.length - 1];
        }
      } catch (error) {
        console.error('Error fetching sensor data:', error);
      }
    };

    onMounted(() => {
      fetchLatestData();
      setInterval(fetchLatestData, 30000); // Fetch data every 30 seconds

      // Handle incoming WebSocket messages
      if (lastMessage.value) {
        const messageData = JSON.parse(lastMessage.value);
        if (messageData.program) {
          currentProgram.value = {
            name: messageData.program,
            status: messageData.status,
            parameters: messageData.parameters,
          };
        }
        if (messageData.sensorData) {
          latestData.value = messageData.sensorData;
        }
      }
    });

    return {
      currentProgram,
      latestData,
      connectionStatus,
      lastMessage,
    };
  },
};
</script>

<style scoped>
.pid-container {
  max-width: 1200px;
  margin: 0 auto;
  padding: 20px;
}

header {
  text-align: center;
  margin-bottom: 40px;
}

.current-program,
.latest-data,
.historical-data {
  margin-bottom: 40px;
}

table {
  width: 100%;
  border-collapse: collapse;
}

th, td {
  border: 1px solid #ddd;
  padding: 8px;
}

th {
  background-color: #f2f2f2;
}
</style>
