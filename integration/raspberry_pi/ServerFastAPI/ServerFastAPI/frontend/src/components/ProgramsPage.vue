<template>
  <div class="programs-container">
    <h1>Programs</h1>
    
    <div class="program-section">
      <h2>Mix Program</h2>
      <input v-model="mixSpeed" type="number" placeholder="Speed (RPM)">
      <button @click="startMix">Start Mix</button>
    </div>

    <div class="program-section">
      <h2>Drain Program</h2>
      <input v-model="drainRate" type="number" placeholder="Rate">
      <input v-model="drainDuration" type="number" placeholder="Duration (seconds)">
      <button @click="startDrain">Start Drain</button>
    </div>

    <div class="program-section">
      <h2>Fermentation Program</h2>
      <input v-model="fermTemp" type="number" placeholder="Temperature">
      <input v-model="fermPH" type="number" placeholder="pH">
      <input v-model="fermDO" type="number" placeholder="Dissolved Oxygen">
      <input v-model="fermNutrient" type="number" placeholder="Nutrient Concentration">
      <input v-model="fermBase" type="number" placeholder="Base Concentration">
      <input v-model="fermDuration" type="number" placeholder="Duration (hours)">
      <input v-model="fermName" type="text" placeholder="Experiment Name">
      <input v-model="fermComment" type="text" placeholder="Comment">
      <button @click="startFermentation">Start Fermentation</button>
    </div>

    <div class="program-section">
      <h2>Stop All Programs</h2>
      <button @click="stopAll">Stop All</button>
    </div>

    <div v-if="message" class="message" :class="{ error: isError }">
      {{ message }}
    </div>

    <div class="websocket-status">
      WebSocket Status: {{ connectionStatus }}
    </div>
    <div v-if="lastMessage" class="last-message">
      Last WebSocket Message: {{ lastMessage }}
    </div>
  </div>
</template>

<script>
import { ref } from 'vue';
import { useWebSocket } from '@/composables/useWebSocket';

export default {
  name: 'ProgramsPage',
  setup() {
    const { sendMessage, lastMessage, connectionStatus } = useWebSocket('ws://192.168.1.25:8000/ws');

    const mixSpeed = ref('');
    const drainRate = ref('');
    const drainDuration = ref('');
    const fermTemp = ref('');
    const fermPH = ref('');
    const fermDO = ref('');
    const fermNutrient = ref('');
    const fermBase = ref('');
    const fermDuration = ref('');
    const fermName = ref('');
    const fermComment = ref('');

    const message = ref('');
    const isError = ref(false);

    const showMessage = (msg, error = false) => {
      message.value = msg;
      isError.value = error;
      setTimeout(() => {
        message.value = '';
        isError.value = false;
      }, 5000);
    };

    const startMix = async () => {
      try {
        const response = await fetch('http://192.168.1.25:8000/execute/mix', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ speed: mixSpeed.value })
        });
        const data = await response.json();
        showMessage(data.message);
        sendMessage({ program: 'mix', speed: mixSpeed.value }); // Example of using sendMessage
      } catch (error) {
        showMessage('Error starting mix program', true);
      }
    };

    const startDrain = async () => {
      try {
        const response = await fetch('http://192.168.1.25:8000/execute/drain', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ rate: drainRate.value, duration: drainDuration.value })
        });
        const data = await response.json();
        showMessage(data.message);
        sendMessage({ program: 'drain', rate: drainRate.value, duration: drainDuration.value });
      } catch (error) {
        showMessage('Error starting drain program', true);
      }
    };

    const startFermentation = async () => {
      try {
        const response = await fetch('http://192.168.1.25:8000/execute/fermentation', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({
            temperature: fermTemp.value,
            pH: fermPH.value,
            dissolvedOxygen: fermDO.value,
            nutrientConcentration: fermNutrient.value,
            baseConcentration: fermBase.value,
            duration: fermDuration.value,
            experimentName: fermName.value,
            comment: fermComment.value
          })
        });
        const data = await response.json();
        showMessage(data.message);
        sendMessage({ program: 'fermentation', ...data });
      } catch (error) {
        showMessage('Error starting fermentation program', true);
      }
    };

    const stopAll = async () => {
      try {
        const response = await fetch('http://192.168.1.25:8000/execute/stop', {
          method: 'POST'
        });
        const data = await response.json();
        showMessage(data.message);
        sendMessage({ program: 'stop' });
      } catch (error) {
        showMessage('Error stopping all programs', true);
      }
    };

    return {
      mixSpeed,
      drainRate,
      drainDuration,
      fermTemp,
      fermPH,
      fermDO,
      fermNutrient,
      fermBase,
      fermDuration,
      fermName,
      fermComment,
      message,
      isError,
      startMix,
      startDrain,
      startFermentation,
      stopAll,
      lastMessage,
      connectionStatus
    };
  }
};
</script>

<style scoped>
.programs-container {
  max-width: 800px;
  margin: 0 auto;
  padding: 20px;
}

.program-section {
  margin-bottom: 30px;
  padding: 20px;
  border: 1px solid #ddd;
  border-radius: 5px;
}

input {
  display: block;
  width: 100%;
  margin-bottom: 10px;
  padding: 5px;
}

button {
  padding: 10px 15px;
  background-color: #4CAF50;
  color: white;
  border: none;
  cursor: pointer;
}

button:hover {
  background-color: #45a049;
}

.message {
  margin-top: 20px;
  padding: 10px;
  border-radius: 5px;
  background-color: #d4edda;
  color: #155724;
}

.message.error {
  background-color: #f8d7da;
  color: #721c24;
}

.websocket-status, .last-message {
  margin-top: 20px;
  padding: 10px;
  background-color: #f0f0f0;
  border-radius: 5px;
}
</style>