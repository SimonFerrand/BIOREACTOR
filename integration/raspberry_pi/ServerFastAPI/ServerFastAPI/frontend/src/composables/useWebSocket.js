import { ref, onMounted, onUnmounted } from 'vue';

export function useWebSocket(url) {
  const socket = ref(null);
  const lastMessage = ref(null);
  const connectionStatus = ref('disconnected');

  const connect = () => {
    socket.value = new WebSocket(url);

    socket.value.onopen = () => {
      connectionStatus.value = 'connected';
      console.log('WebSocket connected');
    };

    socket.value.onmessage = (event) => {
      lastMessage.value = event.data;
      console.log('Received message:', event.data);
    };

    socket.value.onclose = () => {
      connectionStatus.value = 'disconnected';
      console.log('WebSocket disconnected');
      setTimeout(connect, 1000);
    };

    socket.value.onerror = (error) => {
      console.error('WebSocket error:', error);
    };
  };

  const sendMessage = (message) => {
    if (socket.value && socket.value.readyState === WebSocket.OPEN) {
      socket.value.send(JSON.stringify(message));
    } else {
      console.error('WebSocket is not connected');
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
    lastMessage,
    connectionStatus,
    sendMessage
  };
}