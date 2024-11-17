<template>
    <div id="app">
        <BioreactorData />
    </div>
</template>

<script>
import BioreactorData from './components/BioreactorData.vue';

export default {
  name: 'App',
  components: {
    BioreactorData,
  },
};
</script>


<style>
    #app {
        font-family: Avenir, Helvetica, Arial, sans-serif;
        -webkit-font-smoothing: antialiased;
        -moz-osx-font-smoothing: grayscale;
        text-align: center;
        color: #2c3e50;
        margin-top: 60px;
    }
</style>
