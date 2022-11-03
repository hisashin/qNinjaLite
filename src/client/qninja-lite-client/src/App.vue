<template>
  <div id="app">
    <link rel="stylesheet" type="text/css" media="all" href="/css/style-app.css" />
    <TheMain />
  </div>
</template>

<script>
import TheMain from './components/TheMain.vue'
import appState from "./lib/AppState.js";
import device from "./lib/Device.js";

export default {
  name: 'App',
  components: {
    TheMain
  },
  created: function () {
    appState.init();
  },
  computed: {
  }
}
</script>

<style>

</style>
