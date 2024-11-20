import { ref, onMounted, onUnmounted } from 'vue';

export function useWebSocket(url) {
  const socket = ref(null);
  const lastMessage = ref(null);
  const connectionStatus = ref('disconnected');
  const isConnected = ref(false);

  const connect = () => {
    socket.value = new WebSocket(url);

    socket.value.onopen = () => {
      connectionStatus.value = 'connected';
      isConnected.value = true;
      console.log('WebSocket connected');
      
      // Envoyer l'identification
      sendMessage({
        clientType: 'Frontend'
      });
    };

    socket.value.onmessage = (event) => {
      console.log('Raw message received:', event.data);
      try {
        const data = JSON.parse(event.data);
        lastMessage.value = data;
        
        if (data.status === 'disconnected' && data.type === 'ESP32') {
          console.warn('ESP32 disconnected');
        }
      } catch (error) {
        console.warn('Failed to parse message:', error);
        lastMessage.value = event.data;
      }
    };

    socket.value.onclose = () => {
      connectionStatus.value = 'disconnected';
      isConnected.value = false;
      console.log('WebSocket disconnected');
      setTimeout(connect, 5000); // Réessayer dans 5 secondes
    };

    socket.value.onerror = (error) => {
      console.error('WebSocket error:', error);
      isConnected.value = false;
    };
  };

  const sendMessage = (message) => {
    if (socket.value?.readyState === WebSocket.OPEN) {
      const messageString = JSON.stringify(message);
      socket.value.send(messageString);
      console.log('Sent message:', message);
    } else {
      console.error('WebSocket is not connected. Message not sent:', message);
    }
  };

  onMounted(() => {
    connect();
  });

  onUnmounted(() => {
    if (socket.value) {
      socket.value.close();
    }
  });

  return {
    sendMessage,
    lastMessage,
    connectionStatus,
    isConnected
  };
}