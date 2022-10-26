<template>
  <div>
    <header class="header">
      <div class="header__content">
        <h1 class="header__title">
          <a href="ninjaqpcr_splash.html">Ninja qPCR</a>
        </h1>
        <!-- Device monitor -->
          <div class="header__device">
          <span>
            Server:{{ connectionStatus.server.message }}
            Device:{{ connectionStatus.device.message }}
            Well {{ wellTemp }}℃
          </span>
          <!-- Network -->
          <button
            v-show="!connectionStatus.server.connected"
            
            class="btn btn-link"
            @click="reConnect"
          >
            Connect
          </button>
        </div>
      </div>
    </header>
    <main class="main">
      <!--<TheDeviceSummary v-show="selectedPanel!=panels.EXPERIMENT_MONITOR"/>-->
      <nav 
        v-if="panelTitle && selectedPanel!=panels.DASHBOARD"
        class="panel-nav">
        <BackButton v-show="backEnabled" />
        <h2 class="panel-nav__title">
          {{ panelTitle }}
        </h2>
      </nav>
      <!-- Panels -->
      <TheDashboard ref="panelDashboard" v-show="selectedPanel==panels.DASHBOARD" />
      <TheExperimentMonitor ref="panelExperimentMonitor" v-show="selectedPanel==panels.EXPERIMENT_MONITOR" />
      <TheProtocolEditor ref="panelProtocolEditor" v-show="selectedPanel==panels.PROTOCOL_EDITOR" />
    </main>
    <footer class="footer">
    </footer>
  </div>
</template>
<script>

import TheDashboard from './panels/TheDashboard.vue'
import TheExperimentMonitor from './panels/TheExperimentMonitor.vue'
import TheProtocolEditor from './panels/TheProtocolEditor.vue'
import BackButton from './BackButton.vue';
import device from "../lib/Device.js";
import appState from "../lib/AppState.js";

const DEVICE_STATUS_IDLE = 1;
const DEVICE_STATUS_RUNNING = 2;
const DEVICE_STATUS_FINISHED = 3;
const DEVICE_STATUS_AUTO_PAUSE = 4;

export default {
  name: 'TheMain',
  components: {
    TheDashboard,
    TheExperimentMonitor,
    TheProtocolEditor,
    BackButton
  },
  data() {
    return {
      isIdle:true,
      status:DEVICE_STATUS_IDLE,
      panels:appState.PANELS,
      selectedPanel:appState.PANELS.DASHBOARD,
      backEnabled: false,
      panelTitle: "",
      connectionStatus: device.Connection.DISCONNECTED,
      wellTemp: "-",
      initialState:null
    }
  },
  created: function () {
    appState.setPanelContainer(this);
    device.network.connectionStatus.observe((status)=>{
      this.connectionStatus = status;
    });
    device.deviceState.observe((data)=>{
      console.log("TheMain.deviceState")
      console.log(data)
      if (!data) return;
      if (!this.initialState) {
        this.initialState = data
        console.log("Initialstate=%s", data.label)
        if (data.has_experiment && window.confirm("The device has an ongoing experiment. Do you like to open the monitor?")) {
          appState.pushPanel(appState.PANELS.EXPERIMENT_MONITOR, {continue:true});
        }
      }
    });

    device.subscribe("device/update/state", (topic, data)=>{
        if (!this.initialState) {
          this.initialState = data
          console.log("Initialstate=%s", data.label)
          if (data.has_experiment && window.confirm("The device has an ongoing experiment. Do you like to open the monitor?")) {
            appState.pushPanel(appState.PANELS.EXPERIMENT_MONITOR, {continue:true});
          }
        }
    });
    device.subscribe(device.experiment_data_topic("event"), (topic, data)=>{
        console.log("Event received %s", topic)
        console.log("Experiment started.");
        this.status = DEVICE_STATUS_RUNNING;
        // console.log("Experiment is complete!");
        // this.status = DEVICE_STATUS_FINISHED;
    });
    appState.setNavigationHandler((panelStack)=>{
      this.backEnabled = (panelStack.length > 1);
    
    });
  },
  mounted: function () {
    appState.setViews(this.$refs);
  
  },
  methods: {
    reConnect: function () {
      console.log("reConnect");
      device.connect();
    },
    presentPanel(panel, toPanel) {
      this.selectedPanel = panel;
      this.panelTitle = (toPanel.title) ? toPanel.title() : "N/A";
    },
    showTemplate () {
      appState.pushPanel(appState.PANELS.TEMPLATE);
      
    },
    home () {
      appState.home();
    },
    revealDetailLatestExperiment () {
      appState.revealDetailLatestExperiment(this);
    }
  }
}
</script>