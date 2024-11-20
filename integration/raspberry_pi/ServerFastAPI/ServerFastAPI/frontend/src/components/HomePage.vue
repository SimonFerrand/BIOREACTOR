
<template>
  <div class="p-6 space-y-6 bg-gray-900">
    <div class="flex justify-between items-center mb-8">
      <h1 class="text-4xl font-bold text-white">Bioprocess Control Center</h1>
      <div class="flex gap-3 items-center">
        <div class="flex items-center gap-2">
          <div class="h-3 w-3 rounded-full bg-green-500"></div>
          <span class="text-sm text-gray-400">Connected</span>
        </div>
        <div class="flex items-center gap-2">
          <div class="h-3 w-3 rounded-full bg-red-500"></div>
          <span class="text-sm text-gray-400">Offline</span>
        </div>
      </div>
    </div>

    <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
      <div 
        v-for="reactor in bioreactors" 
        :key="reactor.name"
        class="card bg-gray-800 hover:bg-gray-700 transition-all duration-200 cursor-pointer rounded-lg p-6"
        :class="{'border-green-500': reactor.isConnected, 'border-red-500': !reactor.isConnected}"
        @click="handleClick(reactor)"
      >
        <div class="flex items-center justify-between mb-6">
          <h2 class="text-2xl font-bold text-white">{{ reactor.name }}</h2>
          <div 
            class="h-4 w-4 rounded-full animate-pulse"
            :class="[reactor.isConnected ? 'bg-green-500' : 'bg-red-500']"
          ></div>
        </div>
        
        <div v-if="reactor.type === 'heterotrophicXS'" class="grid grid-cols-2 gap-4">
          <div class="flex items-center gap-2 bg-gray-700/50 p-3 rounded-lg">
            <span class="text-blue-400 text-xl">🌡️</span>
            <div>
              <div class="text-lg font-semibold">{{ sensorData.waterTemp || '-' }}°C</div>
              <div class="text-xs text-gray-400">Temperature</div>
            </div>
          </div>
          <div class="flex items-center gap-2 bg-gray-700/50 p-3 rounded-lg">
            <span class="text-purple-400 text-xl">💧</span>
            <div>
              <div class="text-lg font-semibold">{{ sensorData.pH || '-' }}</div>
              <div class="text-xs text-gray-400">pH</div>
            </div>
          </div>
          <div class="flex items-center gap-2 bg-gray-700/50 p-3 rounded-lg">
            <span class="text-yellow-400 text-xl">⚡</span>
            <div>
              <div class="text-lg font-semibold">{{ sensorData.oxygen || '-' }}%</div>
              <div class="text-xs text-gray-400">DO2</div>
            </div>
          </div>
          <div class="flex items-center gap-2 bg-gray-700/50 p-3 rounded-lg">
            <span class="text-green-400 text-xl">🔄</span>
            <div>
              <div class="text-lg font-semibold">{{ sensorData.turbidity || '-' }}</div>
              <div class="text-xs text-gray-400">NTU</div>
            </div>
          </div>
          <div class="col-span-2 mt-2 p-3 bg-gray-700/50 rounded-lg">
            <div class="text-sm text-gray-400">Current Program</div>
            <div class="text-lg font-semibold">{{ sensorData.currentProgram || 'None' }}</div>
          </div>
        </div>

        <div v-else-if="reactor.type === 'phototrophicXS'" class="grid grid-cols-2 gap-4">
          <div class="flex items-center gap-2 bg-gray-700/50 p-3 rounded-lg">
            <span class="text-amber-400 text-xl">💡</span>
            <div>
              <div class="text-lg font-semibold">{{ phototropicData.light ? 'ON' : 'OFF' }}</div>
              <div class="text-xs text-gray-400">Light Status</div>
            </div>
          </div>
          <div class="flex items-center gap-2 bg-gray-700/50 p-3 rounded-lg">
            <span class="text-gray-400 text-xl">🌪️</span>
            <div>
              <div class="text-lg font-semibold">{{ phototropicData.rpm || '-' }}</div>
              <div class="text-xs text-gray-400">RPM</div>
            </div>
          </div>
        </div>

        <div v-else class="text-gray-400 text-center p-4">
          No data available
        </div>
      </div>
    </div>
  </div>
</template>

<script>
export default {
  name: 'HomePage',
  
  data() {
    return {
      sensorData: {},
      phototropicData: {},
      ws: null,
      bioreactors: [
        {
          name: "Heterotrophic XS",
          type: "heterotrophicXS",
          ip: "192.168.1.25",
          isConnected: false
        },
        {
          name: "Phototrophic XS",
          type: "phototrophicXS",
          ip: "192.168.1.26",
          isConnected: false
        },
        {
          name: "Heterotrophic S",
          type: "heterotrophicS",
          ip: "192.168.1.27",
          isConnected: false
        },
        {
          name: "Bain Marie",
          type: "bainMarie",
          ip: "192.168.1.28",
          isConnected: false
        },
        {
          name: "Oven",
          type: "oven",
          ip: "192.168.1.29",
          isConnected: false
        }
      ]
    }
  },

  mounted() {
    this.connectWebSocket();
    this.checkPhototrophicStatus();
    setInterval(this.checkPhototrophicStatus, 30000);
  },

  beforeUnmount() {
    if (this.ws) {
      this.ws.close();
    }
  },

  methods: {
    connectWebSocket() {
      this.ws = new WebSocket('ws://192.168.1.25:8000/ws');
      
      this.ws.onopen = () => {
        console.log('WebSocket Connected');
        this.bioreactors[0].isConnected = true;
      };

      this.ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);
          this.sensorData = data;
        } catch (error) {
          console.error('WebSocket message error:', error);
        }
      };

      this.ws.onclose = () => {
        console.log('WebSocket Disconnected');
        this.bioreactors[0].isConnected = false;
        setTimeout(this.connectWebSocket, 5000);
      };

      this.ws.onerror = (error) => {
        console.error('WebSocket error:', error);
        this.bioreactors[0].isConnected = false;
      };
    },

    async checkPhototrophicStatus() {
      try {
        const response = await fetch('http://192.168.1.26/status');
        if (response.ok) {
          this.phototropicData = await response.json();
          this.bioreactors[1].isConnected = true;
        }
      } catch {
        this.bioreactors[1].isConnected = false;
      }
    },

    handleClick(reactor) {
      if (reactor.type === 'heterotrophicXS') {
        this.$router.push('/charts');
      } else if (reactor.type === 'phototrophicXS') {
        window.open(`http://${reactor.ip}/`, '_blank');
      }
    }
  }
}
</script>

<style scoped>
.card {
  border-width: 2px;
  border-style: solid;
  box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1), 0 2px 4px -1px rgba(0, 0, 0, 0.06);
}

.card:hover {
  transform: translateY(-2px);
  box-shadow: 0 10px 15px -3px rgba(0, 0, 0, 0.1), 0 4px 6px -2px rgba(0, 0, 0, 0.05);
}

.animate-pulse {
  animation: pulse 2s cubic-bezier(0.4, 0, 0.6, 1) infinite;
}

@keyframes pulse {
  0%, 100% {
    opacity: 1;
  }
  50% {
    opacity: .5;
  }
}
</style>
