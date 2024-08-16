<template>
  <div id="app">
    <nav>
      <ul>
        <li><router-link to="/">Home</router-link></li>
        <li><router-link to="/pid">PID</router-link></li>
        <li><router-link to="/charts">Charts</router-link></li>
        <li><router-link to="/programs">Programs</router-link></li>
        <li><router-link to="/settings">Settings</router-link></li>
        <li><router-link to="/calibration">Calibration</router-link></li>
      </ul>
    </nav>
    <router-view/>
  </div>
</template>

<script>
export default {
  name: 'App'
}
</script>

<style scoped>
nav ul {
  list-style-type: none;
  padding: 0;
}
nav li {
  display: inline;
  margin-right: 10px;
}
</style>
