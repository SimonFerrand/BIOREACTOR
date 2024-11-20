<template>
  <div class="programs-container">
    <h1>Programs</h1>
    
    <div class="program-section">
      <h2>Stop All Programs</h2>
      <button @click="stopAll" class="stop-button">Stop All</button>
    </div>

    <div class="program-section">
      <h2>Fermentation Program</h2>
      <div class="form-grid">
        <div class="form-group">
          <label>Temperature (°C)</label>
          <input v-model="fermTemp" type="number" step="0.1" placeholder="Temperature">
        </div>
        
        <div class="form-group">
          <label>pH</label>
          <input v-model="fermPH" type="number" step="0.1" placeholder="pH">
        </div>
        
        <div class="form-group">
          <label>Dissolved Oxygen (%)</label>
          <input v-model="fermDO" type="number" step="0.1" placeholder="DO">
        </div>
        
        <div class="form-group">
          <label>Nutrient Concentration (g/L)</label>
          <input v-model="fermNutrient" type="number" step="0.1" placeholder="Nutrient Concentration">
        </div>
        
        <div class="form-group">
          <label>Base Concentration (mol/L)</label>
          <input v-model="fermBase" type="number" step="0.1" placeholder="Base Concentration">
        </div>
        
        <div class="form-group">
          <label>Duration (hours)</label>
          <input v-model="fermDuration" type="number" placeholder="Duration">
        </div>

        <div class="form-group">
          <label>Nutrient Delay (hours)</label>
          <input v-model="fermNutrientDelay" type="number" step="0.1" placeholder="Nutrient Delay">
        </div>
      </div>
      
      <div class="form-group full-width">
        <label>Experiment Name</label>
        <input v-model="fermName" type="text" placeholder="Experiment Name">
      </div>
      
      <div class="form-group full-width">
        <label>Comment</label>
        <input v-model="fermComment" type="text" placeholder="Comment">
      </div>

      <button @click="startFermentation">Start Fermentation</button>
    </div>

    <div class="program-section">
      <h2>Mix Program</h2>
      <input v-model="mixSpeed" type="number" placeholder="Speed (RPM)">
      <button @click="startMix">Start Mix</button>
    </div>

    <div class="program-section">
      <h2>Drain Program</h2>
      <input v-model="drainRate" type="number" placeholder="Rate (mL/min)">
      <input v-model="drainDuration" type="number" placeholder="Duration (seconds)">
      <button @click="startDrain">Start Drain</button>
    </div>

    <div v-if="message" class="message" :class="{ error: isError }">
      {{ message }}
    </div>

    <div v-if="mqttStatus" class="websocket-status" :class="{ connected: mqttConnected }">
      MQTT Status: {{ mqttStatus }}
    </div>
  </div>
</template>

<script>
import { ref, onMounted } from 'vue';

export default {
  name: 'ProgramsPage',
  setup() {
    const mixSpeed = ref('');
    const drainRate = ref('');
    const drainDuration = ref('');
    const fermTemp = ref('');
    const fermPH = ref('');
    const fermDO = ref('');
    const fermNutrient = ref('');
    const fermBase = ref('');
    const fermDuration = ref('');
    const fermNutrientDelay = ref('');
    const fermName = ref('');
    const fermComment = ref('');

    const message = ref('');
    const isError = ref(false);
    const mqttStatus = ref('Disconnected');
    const mqttConnected = ref(false);

    const showMessage = (msg, error = false) => {
      message.value = msg;
      isError.value = error;
      setTimeout(() => {
        message.value = '';
        isError.value = false;
      }, 5000);
    };

    const startMix = async () => {
      if (!mixSpeed.value) {
        showMessage('Please enter a speed value', true);
        return;
      }
      
      try {
        const response = await fetch('http://192.168.1.25:8000/execute/mix', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ speed: parseInt(mixSpeed.value) })
        });

        const data = await response.json();
        // Vérifier si data existe avant d'accéder à message
        showMessage(data?.message || 'Program started successfully');
      } catch (error) {
        console.error('Error in startMix:', error);
        showMessage('Failed to start mix program', true);
      }
    };

    const startDrain = async () => {
      if (!drainRate.value || !drainDuration.value) {
        showMessage('Please enter both rate and duration values', true);
        return;
      }

      const drainData = {
        rate: parseInt(drainRate.value),
        duration: parseInt(drainDuration.value)
      };

      try {
        const response = await fetch('http://192.168.1.25:8000/execute/drain', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(drainData)
        });
        const data = await response.json();
        showMessage(data.message);
      } catch (error) {
        showMessage('Error starting drain program', true);
      }
    };

    const startFermentation = async () => {
      if (!fermTemp.value || !fermPH.value || !fermDO.value || 
          !fermNutrient.value || !fermBase.value || !fermDuration.value || 
          !fermNutrientDelay.value || !fermName.value) {
        showMessage('Please fill in all required fields', true);
        return;
      }

      const fermentationData = {
        temperature: parseFloat(fermTemp.value),
        pH: parseFloat(fermPH.value),
        dissolvedOxygen: parseFloat(fermDO.value),
        nutrientConcentration: parseFloat(fermNutrient.value),
        baseConcentration: parseFloat(fermBase.value),
        duration: parseInt(fermDuration.value),
        nutrientDelay: parseFloat(fermNutrientDelay.value),
        experimentName: fermName.value,
        comment: fermComment.value || ''
      };

      try {
        const response = await fetch('http://192.168.1.25:8000/execute/fermentation', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(fermentationData)
        });
        const data = await response.json();
        showMessage(data.message);
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
      } catch (error) {
        showMessage('Error stopping all programs', true);
      }
    };

    // Poll MQTT status from backend
    const checkMqttStatus = async () => {
      try {
        const response = await fetch('http://192.168.1.25:8000/mqtt_status');
        const data = await response.json();
        mqttStatus.value = data.status;
        mqttConnected.value = data.connected;
      } catch (error) {
        console.error('Error checking MQTT status:', error);
      }
    };

    onMounted(() => {
      checkMqttStatus();
      setInterval(checkMqttStatus, 5000);
    });

    return {
      mixSpeed, drainRate, drainDuration,
      fermTemp, fermPH, fermDO, fermNutrient,
      fermBase, fermDuration, fermNutrientDelay,
      fermName, fermComment,
      message, isError,
      mqttStatus, mqttConnected,
      startMix, startDrain, startFermentation, stopAll
    };
  }
};
</script>

<style scoped>
.programs-container {
  max-width: 800px;
  margin: 0 auto;
  padding: 20px;
  min-height: 100%;
  overflow-y: auto;
}

.program-section {
  margin-bottom: 30px;
  padding: 20px;
  border: 1px solid var(--dark-surface-hover);
  border-radius: 8px;
  background-color: var(--dark-surface);
}

.form-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 1rem;
  margin-bottom: 1rem;
}

.form-group {
  display: flex;
  flex-direction: column;
  gap: 0.5rem;
}

.form-group.full-width {
  grid-column: 1 / -1;
}

label {
  font-size: 0.875rem;
  color: var(--dark-text-secondary);
}

input {
  width: 100%;
  padding: 0.5rem;
  border: 1px solid var(--dark-surface-hover);
  border-radius: 4px;
  background-color: var(--dark-bg);
  color: var(--dark-text);
  box-sizing: border-box;
}

input:focus {
  outline: none;
  border-color: var(--accent-color);
}

button {
  width: 100%;
  padding: 0.75rem;
  margin-top: 1rem;
  background-color: var(--accent-color);
  color: white;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  transition: background-color 0.2s, transform 0.1s;
}

button:hover {
  filter: brightness(110%);
}

button:active {
  transform: translateY(1px);
}

.stop-button {
  background-color: #f44336;
}

.stop-button:hover {
  filter: brightness(110%);
}

.message {
  margin-top: 20px;
  padding: 10px;
  border-radius: 4px;
  background-color: #4CAF50;
  color: white;
}

.message.error {
  background-color: #f44336;
}

.websocket-status {
  margin-top: 20px;
  padding: 10px;
  background-color: var(--dark-surface);
  border-radius: 4px;
  font-size: 0.875rem;
  color: var(--dark-text-secondary);
  border-left: 4px solid #f44336;
}

.websocket-status.connected {
  border-left-color: #4CAF50;
}
</style>